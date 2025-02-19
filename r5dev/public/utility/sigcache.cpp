//===========================================================================//
//
// Purpose: Implementation of the CSigCache class.
// 
//===========================================================================//
// sigcache.cpp
// 
// The system creates a static cache file on the disk, who's blob contains a 
// map of string signatures and its precomputed relative virtual address.
// 
// This file gets loaded and parsed during DLL init. If the file is absent or 
// outdated/corrupt, the system will generate a new cache file if enabled.
// 
// By caching the relative virtual addresses, we can drop a significant amount 
// of time initializing the DLL by parsing the precomputed data instead of 
// searching for each signature in the memory region of the target executable.
//
///////////////////////////////////////////////////////////////////////////////
#include "core/stdafx.h"
#include "public/utility/binstream.h"
#include "public/utility/sigcache.h"

//-----------------------------------------------------------------------------
// Purpose: whether or not to disable the caching of signatures
// Input  : bDisabled - (true = disabled)
//-----------------------------------------------------------------------------
void CSigCache::SetDisabled(const bool bDisabled)
{
	m_bDisabled = bDisabled;
}

//-----------------------------------------------------------------------------
// Purpose: clears the signature cache memory
//-----------------------------------------------------------------------------
void CSigCache::InvalidateMap()
{
	if (m_bDisabled)
	{
		return;
	}

	(*m_Cache.mutable_smap()).clear();
}

//-----------------------------------------------------------------------------
// Purpose: creates a map of a pattern and relative virtual address
// Input  : &svPattern - (key)
//			nRVA - (value)
//-----------------------------------------------------------------------------
void CSigCache::AddEntry(const string& svPattern, const uint64_t nRVA)
{
	if (m_bDisabled)
	{
		return;
	}

	Assert(!m_bInitialized);
	(*m_Cache.mutable_smap())[svPattern] = nRVA;
}

//-----------------------------------------------------------------------------
// Purpose: finds a pattern key in the cache map and sets its value to nRVA
// Input  : &svPattern - 
//			&nRVA - 
// Output : true if key is found, false otherwise
//-----------------------------------------------------------------------------
bool CSigCache::FindEntry(const string& svPattern, uint64_t& nRVA) const
{
	if (!m_bDisabled && m_bInitialized)
	{
		google::protobuf::Map sMap = m_Cache.smap();
		auto p = sMap.find(svPattern);

		if (p != sMap.end())
		{
			nRVA = p->second;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: loads the cache map from the disk
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CSigCache::LoadCache(const string& svCacheFile)
{
	Assert(!m_bInitialized); // Recursive load.

	if (m_bDisabled)
	{
		return false;
	}

	CIOStream reader(svCacheFile, CIOStream::Mode_t::READ);
	if (!reader.IsReadable())
	{
		return false;
	}
	if (reader.GetSize() <= sizeof(SigDBHeader_t))
	{
		return false;
	}

	SigDBHeader_t header;
	header.m_nMagic = reader.Read<int>();

	if (header.m_nMagic != SIGDB_MAGIC)
	{
		return false;
	}

	header.m_nMajorVersion = reader.Read<uint16_t>();
	if (header.m_nMajorVersion != SIGDB_MAJOR_VERSION)
	{
		return false;
	}

	header.m_nMinorVersion = reader.Read<uint16_t>();
	if (header.m_nMinorVersion != SIGDB_MINOR_VERSION)
	{
		return false;
	}

	header.m_nBlobSizeMem = reader.Read<uint64_t>();
	header.m_nBlobSizeDisk = reader.Read<uint64_t>();
	header.m_nBlobChecksum = reader.Read<uint32_t>();

	uint32_t nAdler32;

	std::unique_ptr<uint8_t[]> pSrcBuf(new uint8_t[header.m_nBlobSizeDisk]);
	std::unique_ptr<uint8_t[]> pDstBuf(new uint8_t[header.m_nBlobSizeMem]);

	reader.Read<uint8_t>(*pSrcBuf.get(), header.m_nBlobSizeDisk);
	DecompressBlob(header.m_nBlobSizeDisk, header.m_nBlobSizeMem, nAdler32, pSrcBuf.get(), pDstBuf.get());

	if (header.m_nBlobChecksum != nAdler32)
	{
		return false;
	}

	if (!m_Cache.ParseFromArray(pDstBuf.get(), header.m_nBlobSizeMem))
	{
		return false;
	}

	m_bInitialized = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: writes the cache map to the disk
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CSigCache::WriteCache(const string& svCacheFile) const
{
	if (m_bDisabled || m_bInitialized)
	{
		// Only write when we don't have anything valid on the disk.
		return false;
	}

	CIOStream writer(svCacheFile, CIOStream::Mode_t::WRITE);
	if (!writer.IsWritable())
	{
		Error(eDLL_T::COMMON, NO_ERROR, "Failed to write cache file: (read-only?)\n");
		return false;
	}

	SigDBHeader_t header;
	header.m_nMagic = SIGDB_MAGIC;
	header.m_nMajorVersion = SIGDB_MAJOR_VERSION;
	header.m_nMinorVersion = SIGDB_MINOR_VERSION;

	const string svBuffer = m_Cache.SerializeAsString();
	std::unique_ptr<uint8_t[]> pBuffer(new uint8_t[svBuffer.size()]);

	header.m_nBlobSizeMem = svBuffer.size();
	uint64_t nCompSize = svBuffer.size();

	if (!CompressBlob(svBuffer.size(), nCompSize, header.m_nBlobChecksum, reinterpret_cast<const uint8_t*>(svBuffer.data()), pBuffer.get()))
	{
		return false;
	}

	header.m_nBlobSizeDisk = nCompSize;

	writer.Write(header);
	writer.Write(pBuffer.get(), nCompSize);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: decompresses the blob containing the signature map
// Input  : nSrcLen - 
//			&nDstLen - 
//			&nAdler - 
//			*pSrcBuf - 
//			*pDstBuf - 
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CSigCache::DecompressBlob(const size_t nSrcLen, size_t& nDstLen, uint32_t& nAdler, const uint8_t* pSrcBuf, uint8_t* pDstBuf) const
{
	lzham_decompress_params lzDecompParams{};
	lzDecompParams.m_dict_size_log2 = SIGDB_DICT_SIZE;
	lzDecompParams.m_decompress_flags = lzham_decompress_flags::LZHAM_DECOMP_FLAG_OUTPUT_UNBUFFERED | lzham_decompress_flags::LZHAM_DECOMP_FLAG_COMPUTE_ADLER32;
	lzDecompParams.m_struct_size = sizeof(lzham_decompress_params);

	lzham_decompress_status_t lzDecompStatus = lzham_decompress_memory(&lzDecompParams, pDstBuf, &nDstLen, pSrcBuf, nSrcLen, &nAdler);

	if (lzDecompStatus != lzham_decompress_status_t::LZHAM_DECOMP_STATUS_SUCCESS)
	{
		Error(eDLL_T::COMMON, NO_ERROR, "Failed to decompress blob: status = %08x\n", lzDecompStatus);
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: compresses the blob containing the signature map
// Input  : nSrcLen - 
//			&nDstLen - 
//			&nAdler - 
//			*pSrcBuf - 
//			*pDstBuf - 
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CSigCache::CompressBlob(const size_t nSrcLen, size_t& nDstLen, uint32_t& nAdler, const uint8_t* pSrcBuf, uint8_t* pDstBuf) const
{
	lzham_compress_params lzCompParams{};
	lzCompParams.m_dict_size_log2 = SIGDB_DICT_SIZE;
	lzCompParams.m_level = lzham_compress_level::LZHAM_COMP_LEVEL_FASTEST;
	lzCompParams.m_compress_flags = lzham_compress_flags::LZHAM_COMP_FLAG_DETERMINISTIC_PARSING;

	lzham_compress_status_t lzCompStatus = lzham_compress_memory(&lzCompParams, pDstBuf, &nDstLen, pSrcBuf, nSrcLen, &nAdler);

	if (lzCompStatus != lzham_compress_status_t::LZHAM_COMP_STATUS_SUCCESS)
	{
		Error(eDLL_T::COMMON, NO_ERROR, "Failed to compress blob: status = %08x\n", lzCompStatus);
		return false;
	}

	return true;
}

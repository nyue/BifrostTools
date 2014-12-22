#ifndef BIF2PRT
#define BIF2PRT

#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_channel.h>
#include <bifrostapi/bifrost_tiledata.h>
#include <bifrostapi/bifrost_types.h>
#include <bifrostapi/bifrost_layout.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <zlib.h>
#include <assert.h>
#include <string.h>

//*************************************************************************
/*! \class PRTConverter bif2prt.h
	\brief %PRTConverter class. This is a sample to demonstrate how to create
	.PRT files (using PRT v1 format) from .BIF files.

	For more details on the prt format used in bif2prt:
	http://www.thinkboxsoftware.com/krak-prt-file-format/
*/
//*************************************************************************
class PRTConverter
{
	public:
	typedef std::pair<Bifrost::API::String /*bifrost*/,std::string/*prt*/> Pair;
	typedef std::vector< Pair > ChannelPairNames;
	static const unsigned int PRT_VERSION = 1;

	//*************************************************************************
	/*! \struct Header bif2prt.h
		\brief %Header structure. Handles the main header section.
	*/
	//*************************************************************************
	struct Header
	{
		static const unsigned int MAGICLEN = 8;
		static const unsigned int SIGLEN = 32;
		static const unsigned int STRUCTSIZE = 56;

		unsigned char _magic[MAGICLEN];
		unsigned int _size;
		unsigned char _sig[SIGLEN];
		unsigned int _version;
		unsigned long long _count;

		/*! Default constructor. */
		Header()
		{
			static const unsigned char magic[MAGICLEN] = {192, 'P', 'R', 'T', '\r', '\n', 26, '\n'};
			static const char sig[] = "Extensible Particle Format";

			memcpy( _magic, magic, sizeof(magic) );
			strncpy( (char*)_sig, sig, SIGLEN );
			_sig[SIGLEN-1] = '\0';
			_size = STRUCTSIZE;
			_version = PRT_VERSION;
			_count = -1; // to be filled later
		}

		/*! Write a %Header object to a file stream. */
		bool write( std::fstream& out, unsigned long long count )
		{
			_count = count;
			out.write( (char*)this, sizeof(Header) );
			return out.good();
		}

	};

	//*************************************************************************
	/*! \struct ChannelDefContainer bif2prt.h
		\brief %ChannelDefContainer structure. Handles the channel definition section.
	*/
	//*************************************************************************
	struct ChannelDefContainer
	{
		//*************************************************************************
		/*! \struct Entry bif2prt.h
			\brief %Entry structure. Holds a channel definition.
		*/
		//*************************************************************************
		struct Entry
		{
			//*************************************************************************
			/*! \struct Header bif2prt.h
				\brief %Header structure. Defines a channel definition data structure.
			*/
			//*************************************************************************
			struct Header
			{
				static const unsigned int NAMELEN = 32;
				static const unsigned int STRUCTSIZE = 44;

				unsigned char _name[NAMELEN];
				unsigned int _type;
				unsigned int _arity;
				unsigned int _offset;

				/*! Default constructor. */
				Header() : _type(0), _arity(0), _offset(0)
				{
				}

				/*! Write a %Header object to a file stream. */
				bool write( std::fstream& out )
				{
					out.write( (char*)this, sizeof(Header) );
					return out.good();
				}
			};

			/*! Default constructor. */
			Entry()
			{
			}

			Header _header;
			Bifrost::API::Channel _channel;
		};

		/*! \p ChannelDefs Array of %Entry channel objects.*/
		typedef std::vector<ChannelDefContainer::Entry> ChannelDefs;

		/*! array of channel definitions. */
		ChannelDefs _channelDefs;

		/*! Number of elements (particles) to save. */
		size_t _numElements;

		/*! the layout we use for iterating channel tiles. */
		Bifrost::API::Layout _layout;

		/*! Default constructor. */
		ChannelDefContainer() : _numElements(0)
		{
		}

		/*! Default destructor. */
		~ChannelDefContainer()
		{
			clear();
		}

		/*! Clears all channel definitions. */
		void clear()
		{
			_channelDefs.clear();
			_layout.reset();
		}

		/*! Returns true if we have channel definitions. */
		bool valid() const
		{
			return _channelDefs.size() > 0;
		}

		/*! Adds new entry in the container. */
		void addEntry( const std::string& prtchname, const Bifrost::API::Channel& ch, int& offset /*in bytes*/ )
		{
			switch (ch.dataType()) {
				case Bifrost::API::DataType::FloatType:
				case Bifrost::API::DataType::FloatV3Type: {
					ChannelDefContainer::Entry def;
					strncpy( (char*)def._header._name, prtchname.c_str(), ChannelDefContainer::Entry::Header::NAMELEN );
					def._header._name[ChannelDefContainer::Entry::Header::NAMELEN-1] = '\0';
					def._header._type = 4; // PRT type
					def._header._arity = (unsigned int)ch.arity();
					def._header._offset = offset;
					def._channel = ch;
					_channelDefs.push_back(def);
					_numElements = ch.elementCount();

					offset += (unsigned int)ch.stride();
				}
				break;

				default: {
					std::cerr << "ChannelDefContainer::addEntry: unsupported type" << std::endl;
				}
			}
		}

		/*! Write all %Entry objects to a file stream. */
		bool write( std::fstream& out )
		{
			if (!valid()) {
				return false;
			}

			// write number of channels
			unsigned int nch = (unsigned int)_channelDefs.size();
			out.write( (char*)&nch, sizeof(unsigned int) );

			// write size of channel header structure
			unsigned int hsize = ChannelDefContainer::Entry::Header::STRUCTSIZE;
			out.write( (char*)&hsize, sizeof(unsigned int) );

			// write channel headers
			for ( size_t i=0; i<_channelDefs.size(); i++ ) {
				_channelDefs[i]._header.write( out );
			}
			return out.good();
		}
	};

	//*************************************************************************
	/*! \struct Reserved bif2prt.h
		\brief %Reserved  structure. Simple handling of reserved bytes.
	*/
	//*************************************************************************
	struct Reserved
	{
		// reserved section
		static const unsigned int RESERVED = 4;
		unsigned int _bytes;

		/*! Default constructor. */
		Reserved() : _bytes(RESERVED)
		{
		}

		/*! Write reserved bytes to a file stream. */
		bool write( std::fstream& out )
		{
			out.write( (char*)&_bytes, sizeof(int) );
			return out.good();
		}
	};

	//*************************************************************************
	/*! \struct ChannelDataBlock bif2prt.h
		\brief %ChannelDataBlock structure. Handles the channel data section.
	*/
	//*************************************************************************
	struct ChannelDataBlock
	{
		static const size_t CHUNK = 4096;

		/*! Default constructor. */
		ChannelDataBlock()
		{
		}

		/*! Write all channels compressed data to a file stream. */
		bool write( std::fstream& out, const Bifrost::API::Component& component, const ChannelDefContainer& cont )
		{
			if ( !cont.valid() || !component.valid() ) {
				return false;
			}

			z_stream zst;
			zst.zalloc = Z_NULL;
			zst.zfree = Z_NULL;
			zst.opaque = Z_NULL;
			if ( deflateInit( &zst, Z_DEFAULT_COMPRESSION ) != Z_OK ) {
				std::cerr << "zlib: deflateInit failed" << std::endl;
				return false;
			}

			std::vector< std::pair<const unsigned char*,size_t> > tileDataPtrs(cont._channelDefs.size());

			char compbuf[CHUNK];
			for ( Bifrost::API::TreeIndex::Depth d = 0; d<cont._layout.depthCount(); d++ ) {
				for ( Bifrost::API::TreeIndex::Tile t = 0; t<cont._layout.tileCount(d); t++ ) {
					Bifrost::API::TreeIndex tindex(t,d);

					// get number of elements at tindex.
					size_t elementCount = component.elementCount( tindex );
					if ( elementCount ) {
						// collect tile data arrays at location tindex
						for (size_t chindex=0; chindex<cont._channelDefs.size(); chindex++ ) {
							size_t bufferSize;
							const ChannelDefContainer::Entry& chdef = cont._channelDefs[chindex];
							std::pair<const unsigned char*,size_t> valuepair;
							valuepair.first = (const unsigned char*)chdef._channel.tileDataPtr( tindex, bufferSize );							
							valuepair.second = chdef._channel.stride();
							tileDataPtrs[chindex] = valuepair;
						}
					}

					// Save the tile data based on this PRT format schema 
					// E.g.
					// [float32][float32][float32][float32][float32][float32][float32][float32][float32][float32][float32][float32]...
					// |_________________________||_________________________||_________________________||_________________________|
					// |		 position                   velocity                  position                    velocity
					// |____________________________________________________||____________________________________________________|
					// 					  particle 1                                            particle 2
					for ( size_t i=0; i<elementCount; i++ ) {
						for (size_t chindex=0; chindex<tileDataPtrs.size(); chindex++ ) {
							const unsigned char* buffer = tileDataPtrs[chindex].first;
							size_t stride = tileDataPtrs[chindex].second;
							if (!writeCompressedData( out, zst, compbuf, (void*)&buffer[i*stride], stride, Z_NO_FLUSH )) {
								return false;
							}
						}
					}
				}
			}

			// flush remaining bits
			writeCompressedData( out, zst, compbuf, 0, 0, Z_FINISH );
			if ( deflateEnd( &zst ) != Z_OK ) {
				std::cerr<<"zlib: deflateEnd failed"<<std::endl;
				return false;
			}
			return true;
		}

		private:
		/*! Compress a single element to disk. */
		bool writeCompressedData( std::ostream& out, z_stream& zst, char* compbuf, void* buf, size_t size /*in bytes*/, int flush )
		{
			zst.next_in = (Bytef*)buf;
			zst.avail_in = (unsigned int)size;
			while ( zst.avail_in > 0 || flush == Z_FINISH ) {
				// compress buf in chunk of size CHUNK until there is no more data to compress
				zst.next_out = (Bytef*)compbuf;
				zst.avail_out = CHUNK;
				int zstate = deflate( &zst, flush );
				if ( zstate == Z_STREAM_ERROR ) {
					std::cerr << "zlib: compression failed (" << zst.msg << ")" << std::endl;
					return false;
				}
				// write compressed buffer to file
				unsigned long long compsize = (unsigned long long)(zst.next_out - (Bytef*)compbuf);
				out.write( (char*)compbuf, compsize );

				if ( zstate==Z_STREAM_END ) {
					break;
				}
			}
			return out.good();
		}

	};

	Header _header;
	Reserved _reserved;
	ChannelDefContainer _channelDefs;
	ChannelDataBlock _channelData;
	std::fstream _fstream;

	/*! Default constructor. */
	PRTConverter()
	{
	}

	/*! Write all sections to a file stream. */
	bool write(	const std::string& out,						/* prt output file */
				const Bifrost::API::Component& component,	/* bifrost component API */
				const ChannelPairNames& chnames				/* channels to export */ )
	{
		if ( _fstream ) {
			_fstream.close();
		}

		_fstream.open( out.c_str(), std::fstream::out|std::fstream::binary );
		if ( !_fstream ) {
			return false;
		}

		_channelDefs.clear();
		_channelDefs._layout = component.layout();

		// collect channel info
		int offset = 0;
		for (size_t i=0; i<chnames.size(); ++i ) {
			const Bifrost::API::Channel& ch = component.findChannel( chnames[i].first );
			_channelDefs.addEntry( chnames[i].second, ch, offset );			
		}

		// dump to disk
		_header.write( _fstream, _channelDefs._numElements );
		_reserved.write( _fstream );
		_channelDefs.write( _fstream );
		_channelData.write( _fstream, component, _channelDefs );

		bool state = _fstream.good();
		_fstream.close();
		return state;
	}
};

#endif

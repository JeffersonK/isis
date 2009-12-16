#include <DataStorage/io_interface.h>

namespace isis{ namespace image_io{ 

class ImageFormat_Null: public FileFormat{
public:
	std::string suffixes(){
		return std::string(".null .null.gz");
	}
	std::string dialects(){
		return std::string("inverted");
	}
	std::string name(){
		return "Null";
	}
  
	data::ChunkList load (std::string filename,std::string dialect ){
		
		const size_t images=5;
		const size_t timesteps=10;

		data::ChunkList chunks;
		for(int i=0;i<timesteps;i++){
			for(int c=0;c<images;c++){
				data::MemChunk<short> ch(3,3,3);
				ch.setProperty("indexOrigin",util::fvector4(0,0,0,i));
				ch.setProperty("acquisitionNumber",c);
				ch.setProperty("sequenceNumber",c);
				ch.setProperty("readVec",util::fvector4(1,0));
				ch.setProperty("phaseVec",util::fvector4(0,1));
				ch.setProperty("voxelSize",util::fvector4(1,1,1));
				for(int x=0;x<3;x++)
					ch.voxel<short>(x,x,x)=c+i;
				chunks.push_back(ch);
			}
		}
		return chunks;//return data::ChunkList();
	}
	
	bool write(const data::Image &image,std::string filename,std::string dialect ){
		if(image.size() != util::fvector4(3,3,3,10))return false;
		const int snum=image.getProperty<int>("sequenceNumber");

		std::cout
		<<  "If I was a real writer, I would now write an image with the sequence number "
		<< snum << " and the size "	<< image.sizeToString() << " to " << filename << std::endl;
		
		for(int i=0;i<10;i++)
			if(image.voxel<short>(0,0,0,i) != i+snum)return false;
		return true;
	}
	bool tainted(){return false;}//internal plugins are not tainted
	size_t maxDim(){return 4;}
};
}}
isis::image_io::FileFormat* factory(){
  return new isis::image_io::ImageFormat_Null();
}

/*
 *  imageFormat_SiemensTcpIp.cpp
 *  BARTApplication
 *
 *  Created by Lydia Hellrung on 12/10/10.
 *  Copyright 2010 MPI Cognitive and Human Brain Sciences Leipzig. All rights reserved.
 *
 */


#include <DataStorage/io_interface.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace isis
{
	namespace image_io
	{
		
		
		class ImageFormat_SiemensTcpIp: public FileFormat
		{
			
		protected:
			std::string suffixes()const {
				return std::string( ".tcpip" );
			}
		public:
			std::string name()const {
				return "SiemensTcpIp";
			}
			
			int load ( data::ChunkList &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
				
				firstHeaderArrived = false;
				unsigned long data_received = 0L;
                std::string header_start = "<data_block_header>";
                std::string header_end =  "</data_block_header>";
                unsigned int byteSize = 0;
                char* dataBuffer;
                std::string header;
             	
				for(;;){
				memset(buffer, 0, sizeof(buffer));
				unsigned int length = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&receiver_address, &receiver_address_length);
				if(length == 0) {
					exit(0);
				}
                    
                     //TODO: If this was the last volume return something empty, something like:
                    // std::string lastVolume = <lastVolume>
                    // if (memcmp(buffer, lastVolume.c_str(), sizeof(lastVolume) -1 ) == 0 ){
                    //  return 0;}
                    
				
				if(memcmp(buffer, header_start.c_str(), sizeof(header_start) - 1) == 0) {
					firstHeaderArrived = true;
					printf("[receiver2] received block with header\n");
					data_received = 0L;
					unsigned int header_length = 0;
					while(memcmp(header_end.c_str(), (buffer + header_length), sizeof(header_end)) != 0) {
						header_length++;
					}
					header_length += sizeof(header_end);
                    header = std::string(buffer, header_length);
                    
                    byteSize = atoi(getStringFromHeader("data_size_in_bytes", header).c_str());
					
					printf("byteSize: '%i'\n", byteSize);
					dataBuffer = (char*)malloc(byteSize);
					
					// copy the first bit of data
					memcpy(dataBuffer, (buffer + header_length), sizeof(buffer) - header_length);
					
					data_received += sizeof(buffer) - header_length;
					
				} else if (true == firstHeaderArrived) {
					
                   if(byteSize - data_received > 32768) {
						memcpy(dataBuffer + data_received, buffer, sizeof(buffer));
						data_received += sizeof(buffer);
					} else {
						// image complete
                        
                        size_t width = atoi(getStringFromHeader("width", header).c_str());
                        
                        bool moco = getStringFromHeader("motion_corrected", header).compare("yes") == 0 ? true : false;
                        
                        
                        size_t height = atoi(getStringFromHeader("height", header).c_str());
                        bool mosaic = getStringFromHeader("mosaic", header).compare("yes") == 0 ? true : false;
                        size_t iim = 1;
                        //printf("Header \n %s \n", header.c_str());
                        if ( true == mosaic){
                            iim = atoi(getStringFromHeader("images_in_mosaic", header).c_str());
                        }
                        
                        std::string data_type = getStringFromHeader("data_type", header);
                        
                        // Mosaics are always quadratic, so don't bother but only looking for the rows
                        size_t slices_in_row = static_cast<size_t> (ceil(sqrt(static_cast<double_t> (iim) )));
                        size_t width_slice = width / slices_in_row;
                        size_t height_slice = height / slices_in_row;
                        
                        printf("width_slices: %ld height_slices: %ld\n", width_slice, height_slice);
                        printf("data_type '%s'\n", data_type.c_str());
                        
                        size_t acq_nr = atoi(getStringFromHeader("acquisition_number", header).c_str());
                        std::string seq_descr = getStringFromHeader("sequence_description", header);
                        
                        
                        
                        /**********/
                         
                        size_t seq_number = atoi(getStringFromHeader("meas_uid", header).c_str());
                        //printf("series_number %ld\n", series_number );
                        size_t acq_time = atoi(getStringFromHeader("acquisition_time", header).c_str());
                        //printf("acq_time %ld\n", acq_time );
                        uint16_t rep_time = atol(getStringFromHeader("repetition_time", header).c_str());
                        //printf("repetition_time %ld\n", rep_time );
                        std::string read_vector = getStringFromHeader("read_vector", header);
                        size_t indexK1 = read_vector.find(",", 0, 1);
                        size_t indexK2 = read_vector.find(",", indexK1+1, 1);
                        double_t val1 = atof(read_vector.substr(0, indexK1).c_str());
                        double_t val2 = atof(read_vector.substr(indexK1+1, indexK2-indexK1).c_str());
                        double_t val3 = atof(read_vector.substr(indexK1+indexK2, read_vector.length()-indexK2).c_str());
                        util::fvector4 read_vec(val1, val2, val3);
                        //printf("!!!!!!read_vec %.8f,%.8f,%.8f\n", read_vec[0], read_vec[1],read_vec[2] );
                        //printf("read_vector %s\n", read_vector.c_str() );
                        
                        
                        util::fvector4 phase_vec = getVectorFromString(getStringFromHeader("phase_vector", header));
                        //printf("!!!!!!phase_vec %.8f,%.8f,%.8f\n", phase_vec[0], phase_vec[1],phase_vec[2] );
                        //printf("phase_vector %s\n", phase_vector.c_str() );
                        
                        util::fvector4 slice_norm_vec = getVectorFromString(getStringFromHeader("slice_norm_vector", header));
                        //printf("!!!!!!slice_norm_vec %.8f,%.8f,%.8f\n", slice_norm_vec[0], slice_norm_vec[1],slice_norm_vec[2] );
                        //printf("slice_norm_vector %s\n", slice_norm_vector.c_str() );
                        
                        size_t inplane_rot = atoi(getStringFromHeader("implane_rotation", header).c_str());
                        
                        std::string slice_orient = getStringFromHeader("slice_orientation", header);
                        
                        //Fallunterscheidung
                        // Wenn ((slice_orient == TRANSVERSE) gilt: (-45 < inplane_rot < 45)) ? -> COL : ROW -> phaseVec == col + readVec == row
                        // Wenn ((slice_orient != TRANSVERSE) gilt: (-45 < inplane_rot < 45)) ? -> ROW : COL -> phaseVec == row + readVec == col
                        std::string InPlanePhaseEncodingDirection;
                        if (0 == slice_orient.compare(0, slice_orient.length(), "TRANSVERSE")) {
                            InPlanePhaseEncodingDirection = (-45 < inplane_rot && inplane_rot < 45 ) ? "COL" : "ROW";
                        }
                        else {
                            InPlanePhaseEncodingDirection = (-45 < inplane_rot && inplane_rot < 45 ) ? "ROW" : "COL";
                        }

                        
                        size_t fov_read = atoi(getStringFromHeader("fov_read", header).c_str());
                        //printf("fov_read %ld\n", fov_read );
                        
                        size_t fov_phase = atoi(getStringFromHeader("fov_phase", header).c_str());
                        
                        //printf("fov_phase %ld\n", fov_phase );
                        
                        size_t slice_thickness = atoi(getStringFromHeader("slice_thickness", header).c_str());
                        
                        size_t dimension_number = atoi(getStringFromHeader("dimension_number", header).c_str());
                        
                        /**
                         * get voxelGap from first two slice positions
                         */                         
                        //TODO: Was wenn anat Daten
                        std::string slice0_pos = getStringFromHeader("slice_position_0", header);
                        indexK1 = slice0_pos.find(",", 0, 1);
                        indexK2 = slice0_pos.find(",", indexK1+1, 1);
                        double_t slice0 = atof(slice0_pos.substr(indexK1+indexK2, slice0_pos.length()-indexK2).c_str());
                        std::string slice1_pos = getStringFromHeader("slice_position_1", header);
                        indexK1 = slice1_pos.find(",", 0, 1);
                        indexK2 = slice1_pos.find(",", indexK1+1, 1);
                        double_t slice1 = atof(slice1_pos.substr(indexK1+indexK2, slice1_pos.length()-indexK2).c_str());
                        
                        util::fvector4 voxelGap(0, 0, (slice1 - slice0 - slice_thickness));
                        
                        //*********
                        
                        /************/
                        
						image_counter++;
												
						memcpy(dataBuffer + data_received, buffer, byteSize - data_received);
						data_received += byteSize - data_received;
						printf("[receiver2] did receive %i bytes of data\n", byteSize);
						
						// ... do something ...
						/******************************/
					
                       if ( 0 == data_type.compare("short")){
                           char slice_buffer[width_slice * height_slice * sizeof(short)];
                            for(unsigned int _slice = 0; _slice < iim; _slice++) {
                                    for(unsigned int _row = 0; _row < height_slice; _row++) {
                                        char* line_start = dataBuffer + (sizeof(short) * (((_slice / slices_in_row) * (slices_in_row * height_slice * width_slice)) + 
                                                                                          (_row * width_slice * slices_in_row) + (_slice % slices_in_row * width_slice)));

                                        memcpy(slice_buffer + (_row * width_slice * sizeof(short)), line_start, (width_slice * sizeof(short)));
                                    }
                                
                                /********
                                * get each slice position from header 
                                */
                                std::string slice_pos_start = "<slice_position_";
                                char buf[5];
                                sprintf(buf, "%i>", _slice);
                                slice_pos_start.append(buf);
                                slice_pos_start.append("\n");
                                std::string slice_pos_end = "\n</slice_position_";
                                slice_pos_end.append(buf);
                                std::string slice_pos = header.substr((header.find(slice_pos_start)+slice_pos_start.length()), 
                                                                      header.find(slice_pos_end) - (header.find(slice_pos_start)+slice_pos_start.length()));
                                util::fvector4 slice_pos_vec = getVectorFromString(slice_pos);//(val1, val2, val3);
                                printf("Slice string: %s\n\n", slice_pos.c_str());
                                std::cout << "SlicePos: " << slice_pos_vec << std::endl;
                                
                                printf("Read string: %s\n\n", read_vector.c_str());
                                std::cout << "ReadVec: " << read_vec << std::endl;
                                
                                printf("Phase string: %s\n\n",getStringFromHeader("phase_vector", header).c_str());
                                std::cout << "PhaseVec: " << phase_vec << std::endl;
                                //*********
                                
                                boost::shared_ptr<data::Chunk> ch(new data::MemChunk<uint16_t>((uint16_t*)slice_buffer, height_slice,width_slice,1) );
                                ch->setProperty("indexOrigin", util::fvector4(0,0,_slice));
                                
                                ch->setProperty<uint32_t>("acquisitionNumber", acq_nr);
                                if (true == moco){
                                    ch->setProperty<uint16_t>("sequenceNumber", 0);
                                }
                                else {
                                    ch->setProperty<uint16_t>("sequenceNumber", 1);
                                }
                                ch->setProperty<std::string>("sequenceDescription", seq_descr);
                                

                                if ( 0 == InPlanePhaseEncodingDirection.compare(0, 3, "COL") ){
                                    ch->setProperty<util::fvector4>("readVec", phase_vec);
                                    ch->setProperty<util::fvector4>("phaseVec", read_vec);
                                    ch->setProperty<util::fvector4>("voxelSize", util::fvector4(fov_read/width_slice,fov_phase/height_slice,slice_thickness,0));
                                }
                                else {
                                    ch->setProperty<util::fvector4>("phaseVec", phase_vec);
                                    ch->setProperty<util::fvector4>("readVec", read_vec);
                                    ch->setProperty<util::fvector4>("voxelSize", util::fvector4(fov_phase/width_slice,fov_read/height_slice,slice_thickness,0));
                                }

                                
                                ch->setProperty<util::fvector4>("sliceVec", slice_norm_vec);
                                ch->setProperty<uint16_t>("repetitionTime",rep_time);
                                ch->setProperty<std::string>("InPlanePhaseEncodingDirection",InPlanePhaseEncodingDirection);
                                ch->setProperty<util::fvector4>( "voxelGap", voxelGap );
                                chunks.push_back(ch);
                            }
                       }
                        else {
                            printf("DATATYPE NOT SUPPORTED\n");
                        }

				
						
						/******************************/
						free(dataBuffer);
                        /*****************************/
                        // TIME DIFF from send to finished reading data
                        time_t startTime = static_cast<time_t>(atof(getStringFromHeader("time_stamp", header).c_str()));
                        time_t endTime;
                        time(&endTime);
                        double_t dif = difftime (endTime,startTime);
                        printf ("It took us %.2lf seconds to get this volume.\n", dif );
                        
                        
                        
                        
                        
                        
                        
						return 0;
					}
					
				}
                    
				
			}
               
				
				
				
				return 0;
			}
			
			void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
			}
			bool tainted()const {return false;}//internal plugins are not tainted
			
			~ImageFormat_SiemensTcpIp()
			{}
			
			
			
		public:
			static int    sock;
			static struct sockaddr_in receiver_address;
			static int counter;
			static int image_counter;
			
			static unsigned int receiver_address_length;
			static bool firstHeaderArrived;
		private:
			
            std::string getStringFromHeader(const std::string& propName, const std::string& header){
                
                std::string prop_start = "<";
                prop_start.append(propName);
                prop_start.append(">\n");
                std::string prop_end = "\n</";
                prop_end.append(propName);
                prop_end.append(">");
                std::string propString = header.substr((header.find(prop_start)+prop_start.length()), 
                                                         header.find(prop_end) - (header.find(prop_start)+prop_start.length()));
                return propString;
            }
            
            util::fvector4 getVectorFromString(std::string propName)
            {
                size_t indexK1 = propName.find(",", 0, 1);
                size_t indexK2 = propName.find(",", indexK1+1, 1);
                double_t val1 = atof(propName.substr(0, indexK1).c_str());
                double_t val2 = atof(propName.substr(indexK1+1, indexK2-indexK1).c_str());
                double_t val3 = atof(propName.substr(indexK1+indexK2, propName.length()-indexK2).c_str());
                return util::fvector4(val1, val2, val3);
                
            }
            
			char   buffer[32768];
        
        };
		
		int    ImageFormat_SiemensTcpIp::sock;
		struct sockaddr_in ImageFormat_SiemensTcpIp::receiver_address;
		int ImageFormat_SiemensTcpIp::counter;
		int ImageFormat_SiemensTcpIp::image_counter;
		
		unsigned int ImageFormat_SiemensTcpIp::receiver_address_length;
		bool ImageFormat_SiemensTcpIp::firstHeaderArrived;
		
	}
}
isis::image_io::FileFormat *factory()
{
	isis::image_io::ImageFormat_SiemensTcpIp *pluginRtExport = new isis::image_io::ImageFormat_SiemensTcpIp();
	
	pluginRtExport->sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("[receiver2] sock --> %i\n", pluginRtExport->sock);
	
	memset((void*)&pluginRtExport->receiver_address, 0, sizeof(pluginRtExport->receiver_address));
	pluginRtExport->receiver_address.sin_family      = PF_INET;
	pluginRtExport->receiver_address.sin_addr.s_addr = INADDR_ANY;
	pluginRtExport->receiver_address.sin_port        = htons(54321);
		
	printf("[bind] --> %i\n", bind(pluginRtExport->sock, (struct sockaddr*)&pluginRtExport->receiver_address, sizeof(pluginRtExport->receiver_address)));
		
	pluginRtExport->receiver_address_length = sizeof(pluginRtExport->receiver_address);
	pluginRtExport->counter = 0;
	pluginRtExport->image_counter = 0;
	
	
	
	
	return (isis::image_io::FileFormat*) pluginRtExport;
}


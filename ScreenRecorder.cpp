#include "ScreenRecorder.h"

using namespace std;

ScreenRecorder::ScreenRecorder()
{
	cout<<"\n\n Registering required functions...";
	av_register_all();
	avcodec_register_all();
	avdevice_register_all();
	cout<<"\n\n Registered successfully...";
}

ScreenRecorder::~ScreenRecorder()
{

	avformat_close_input(&pAVFormatContext);
	if( !pAVFormatContext )
	{
	cout<<"\n\n1.Success : avformat_close_input()";
	}
	else
	{
	cout<<"\n\nError : avformat_close_input()";
	}

	avformat_free_context(pAVFormatContext);
	if( !pAVFormatContext )
	{
	cout<<"\n\n2.Success : avformat_free_context()";
	}
	else
	{
	cout<<"\n\nError : avformat_free_context()";
	}

cout<<"\n\n---------------Successfully released all resources------------------\n\n\n";
cout<<endl;
cout<<endl;
cout<<endl;
}

int ScreenRecorder::collectFrames()
{
	int flag;
	int frameFinished;
//when you decode a single packet, you still don't have information enough to have a frame [depending on the type of codec, some of them //you do], when you decode a GROUP of packets that represents a frame, then you have a picture! that's why frameFinished will let //you know you decoded enough to have a frame.

	int frame_index = 0;
	value = 0;

	pAVPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_init_packet(pAVPacket);

	pAVFrame = av_frame_alloc();
	if( !pAVFrame )
	{
	 cout<<"\n\nError : av_frame_alloc()";
	 return -1;
	}

	outFrame = av_frame_alloc();//Allocate an AVFrame and set its fields to default values.
	if( !outFrame )
	{
	 cout<<"\n\nError : av_frame_alloc()";
	 return -1;
	}

	int video_outbuf_size;
	int nbytes = av_image_get_buffer_size(outAVCodecContext->pix_fmt,outAVCodecContext->width,outAVCodecContext->height,32);
	uint8_t *video_outbuf = (uint8_t*)av_malloc(nbytes);
	if( video_outbuf == NULL )
	{
	cout<<"\n\nError : av_malloc()";
	}

	// Setup the data pointers and linesizes based on the specified image parameters and the provided array.
	value = av_image_fill_arrays( outFrame->data, outFrame->linesize, video_outbuf , AV_PIX_FMT_YUV420P, outAVCodecContext->width,outAVCodecContext->height,1 ); // returns : the size in bytes required for src
	if(value < 0)
	{
	cout<<"\n\nError : av_image_fill_arrays()";
	}

	SwsContext* swsCtx_ ;

	// Allocate and return swsContext.
	// a pointer to an allocated context, or NULL in case of error
	// Deprecated : Use sws_getCachedContext() instead.
	swsCtx_ = sws_getContext(pAVCodecContext->width,
		                pAVCodecContext->height,
		                pAVCodecContext->pix_fmt,
		                outAVCodecContext->width,
				outAVCodecContext->height,
		                outAVCodecContext->pix_fmt,
		                SWS_BICUBIC, NULL, NULL, NULL);


int ii = 0;
int no_frames = 100;
cout<<"\n\nEnter No. of Frames to capture : ";
cin>>no_frames;

	AVPacket outPacket;
	int j = 0;

	int got_picture;

	while( av_read_frame( pAVFormatContext , pAVPacket ) >= 0 )
	{
	if( ii++ == no_frames )break;
		if(pAVPacket->stream_index == VideoStreamIndx)
		{
			value = avcodec_decode_video2( pAVCodecContext , pAVFrame , &frameFinished , pAVPacket );
			if( value < 0)
			{
				cout<<"Error : avcodec_decode_video2()";
			}

			if(frameFinished)// Frame successfully decoded :)
			{
				sws_scale(swsCtx_, pAVFrame->data, pAVFrame->linesize,0, pAVCodecContext->height, outFrame->data,outFrame->linesize);
				av_init_packet(&outPacket);
				outPacket.data = NULL;    // packet data will be allocated by the encoder
				outPacket.size = 0;

				avcodec_encode_video2(outAVCodecContext , &outPacket ,outFrame , &got_picture);

				if(got_picture)
				{
					if(outPacket.pts != AV_NOPTS_VALUE)
						outPacket.pts = av_rescale_q(outPacket.pts, video_st->codec->time_base, video_st->time_base);
					if(outPacket.dts != AV_NOPTS_VALUE)
						outPacket.dts = av_rescale_q(outPacket.dts, video_st->codec->time_base, video_st->time_base);
				
					printf("Write frame %3d (size= %2d)\n", j++, outPacket.size/1000);
					if(av_write_frame(outAVFormatContext , &outPacket) != 0)
					{
						cout<<"\n\nError : av_write_frame()";
					}

				av_packet_unref(&outPacket);
				} // got_picture

			av_packet_unref(&outPacket);
			} // frameFinished

		}
	}// End of while-loop

	value = av_write_trailer(outAVFormatContext);
	if( value < 0)
	{
		cout<<"\n\nError : av_write_trailer()";
	}


//THIS WAS ADDED LATER
av_free(video_outbuf);

}

int ScreenRecorder::openCamera()
{

	value = 0;
	options = NULL;
	pAVFormatContext = NULL;

	pAVFormatContext = avformat_alloc_context();//Allocate an AVFormatContext.

	pAVInputFormat = av_find_input_format("x11grab");
  value = avformat_open_input(&pAVFormatContext, ":0.0+10,250", pAVInputFormat, NULL);
	if(value != 0)
	{
	   cout<<"\n\nError : avformat_open_input\n\nstopped...";
	   return -1;
	}

	value = av_dict_set( &options,"framerate","30",0 );
	if(value < 0)
	{
	  cout<<"\n\nError : av_dict_set(framerate , 30 , 0)";
    return -1;
	}

	value = av_dict_set( &options, "preset", "medium", 0 );
	if(value < 0)
	{
	  cout<<"\n\nError : av_dict_set(preset , medium)";
	      return -1;
	}

//	value = avformat_find_stream_info(pAVFormatContext,NULL);
	if(value < 0)
	{
	  cout<<"\n\nError : avformat_find_stream_info\nstopped...";
	  return -1;
	}

	VideoStreamIndx = -1;

	for(int i = 0; i < pAVFormatContext->nb_streams; i++ ) // find video stream posistion/index.
	{
	  if( pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
	  {
	     VideoStreamIndx = i;
	     break;
	  }

	} // End for-loop

	if( VideoStreamIndx == -1)
	{
	  cout<<"\n\nError : VideoStreamIndx = -1";
	  return -1;
	}

	// assign pAVFormatContext to VideoStreamIndx
	pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec;

	pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
	if( pAVCodec == NULL )
	{
	  cout<<"\n\nError : avcodec_find_decoder()";
	  return -1;
	}

	value = avcodec_open2(pAVCodecContext , pAVCodec , NULL);//Initialize the AVCodecContext to use the given AVCodec.
	if( value < 0 )
	{
	  cout<<"\n\nError : avcodec_open2()";
	  return -1;
	}
}

int ScreenRecorder::init_outputfile()
{
	outAVFormatContext = NULL;
	value = 0;
	output_file = "output.mp4";

	avformat_alloc_output_context2(&outAVFormatContext, NULL, NULL, output_file);
	if (!outAVFormatContext)
	{
		cout<<"\n\nError : avformat_alloc_output_context2()";
	  return -1;
	}

/*Returns the output format in the list of registered output formats which best matches the provided parameters, or returns NULL if there is no match.
*/
	output_format = av_guess_format(NULL, output_file ,NULL);
	if( !output_format )
	{
	 cout<<"\n\nError : av_guess_format()";
	 return -1;
	}

	video_st = avformat_new_stream(outAVFormatContext ,NULL);
	if( !video_st )
	{
		cout<<"\n\nError : avformat_new_stream()";
	  return -1;
	}

	outAVCodecContext = avcodec_alloc_context3(outAVCodec);
	if( !outAVCodecContext )
	{
	  cout<<"\n\nError : avcodec_alloc_context3()";
	  return -1;
	}

	outAVCodecContext = video_st->codec;
	outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
	outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
	outAVCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;
	outAVCodecContext->bit_rate = 400000; // 2500000
	outAVCodecContext->width = 1920;
	outAVCodecContext->height = 1080;
	outAVCodecContext->gop_size = 3;
	outAVCodecContext->max_b_frames = 2;
	outAVCodecContext->time_base.num = 1;
	outAVCodecContext->time_base.den = 30; // 15fps

	if (codec_id == AV_CODEC_ID_H264)
	{
	 av_opt_set(outAVCodecContext->priv_data, "preset", "slow", 0);
	}

	outAVCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
	if( !outAVCodec )
	{
	 cout<<"\n\nError : avcodec_find_encoder()";
	 return -1;
	}

// Some container formats (like MP4) require global headers to be present
// Mark the encoder so that it behaves accordingly.

	if ( outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
	{
		outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	value = avcodec_open2(outAVCodecContext, outAVCodec, NULL);
	if( value < 0)
	{
		cout<<"\n\nError : avcodec_open2()";
		return -1;
	}

	if ( !(outAVFormatContext->flags & AVFMT_NOFILE) )
	{
	 if( avio_open2(&outAVFormatContext->pb , output_file , AVIO_FLAG_WRITE ,NULL, NULL) < 0 )
	 {
	  cout<<"\n\nError : avio_open2()";
	 }
	}



	if(!outAVFormatContext->nb_streams)
	{
		cout<<"\n\nError : Output file dose not contain any stream";
	  return -1;
	}

	value = avformat_write_header(outAVFormatContext , &options);
	if(value < 0)
	{
		cout<<"\n\nError : avformat_write_header()";
		return -1;
	}

	cout<<"\n\nOutput file information :\n\n";
	av_dump_format(outAVFormatContext , 0 ,output_file ,1);
}

int main()
{
	ScreenRecorder s_record;

	s_record.openCamera();
	s_record.init_outputfile();
	s_record.collectFrames();

	cout<<"\n\n---------EXIT_SUCCESS------------\n\n";

return 0;
}

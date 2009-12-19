set(ccrtp1_headers base.h 
		 formats.h 
		 rtppkt.h 
		 rtcppkt.h 
		 sources.h 
		 channel.h  
		 queuebase.h 
		 iqueue.h 
		 oqueue.h
		 ioqueue.h
		 cqueue.h
		 ext.h
		 rtp.h 
		 pool.h
		 CryptoContext.h)

include(SourceDistribution)
src_distribution_init()
add_src_dist_dirs(crypto)
add_src_dist_files(${ccrtp1_headers})

add_subdirectory(crypto)

########### install files ###############

install(FILES ${ccrtp1_headers} DESTINATION include/ccrtp)

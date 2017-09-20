FROM ubuntu:16.04

MAINTAINER john@tastemade.com

# RUN apt-get install yasm
RUN apt-get update && \
	apt-get install -y git

RUN git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg && \
 	cd ffmpeg && \
	git checkout release/2.1

RUN apt-get install -y yasm gcc
RUN apt-get install -y make

RUN cd ffmpeg && \
	./configure --enable-shared && \
	make && \
	make install

RUN echo /usr/local/lib >> /etc/ld.so.conf && \
	ldconfig

RUN apt-get install -y curl unzip

RUN curl http://cimg.eu/files/CImg_2.0.2.zip > cimg.zip && \
	unzip cimg.zip && \
	rm cimg.zip && \
	mv CImg-2.0.2 cimg && \
	cp cimg/CImg.h /usr/local/include/

ADD bindings ./pHash/bindings/
ADD examples ./pHash/examples/
ADD m4 ./pHash/m4/
ADD src ./pHash/src/
ADD * ./pHash/

RUN apt-get install -y yasm g++

RUN cd pHash && \ 
    cp /usr/local/include/CImg.h ./ && \
	./configure LIBS='-lavcodec -lavutil' --enable-audio-hash=no && \
	make

RUN cd src && make
RUN cd bindings && make
RUN cd examples && make

CMD ["/bin/bash"]


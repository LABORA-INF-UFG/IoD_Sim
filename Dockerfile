FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

####
# Machine Setup
####
RUN apt-get update \
    && apt-get install --no-install-recommends --quiet --assume-yes \
        git \
        gcc \
        g++ \
        gdb \
        valgrind \
        patch \
        ssh \
        sudo \
        # ns3 dependencies
        python3 \
        python-is-python3 \
        python3-dev \
        python3-pip \
        python3-pybindgen \
        python3-pygccxml \
        pkg-config \
        sqlite3 \
        #libgtk-3-dev \
        #libsqlite3-dev \
        # pybindgen dependencies
        #libboost-dev \
        #castxml \
        # IoD_Sim dependencies
        rapidjson-dev \
        libgsl-dev \
        libxml2-dev \
        # Fork dependencies
        git

# Install IoD_Sim
# Dependencies
RUN ./tools/install-dependencies.sh

#RUN git clone https://github.com/telematics-lab/IoD_Sim

WORKDIR /IoD_Sim
COPY . .

# Config scripts
#RUN ./tools/prepare-ns3.sh
#RUN cd ns3/ \
#    && ./ns3 configure --build-profile=debug --enable-examples --disable-mpi --disable-python --enable-modules=iodsim \
#    && ./ns3 build

FROM arduino-cli

RUN pacman -Sy --noconfirm base-devel
RUN git clone https://github.com/khoih-prog/functional-vlpp ~/Arduino/libraries/Functional-VLPP
RUN git clone https://github.com/khoih-prog/Ethernet2 ~/Arduino/libraries/Ethernet2
RUN git clone https://github.com/khoih-prog/EthernetWebServer ~/Arduino/libraries/EthernetWebServer
RUN git clone https://github.com/knolleary/pubsubclient/ ~/Arduino/libraries/pubsubclient
RUN git clone https://github.com/finitespace/BME280 ~/Arduino/libraries/BME280
RUN git clone https://github.com/reaper7/SDM_Energy_Meter ~/Arduino/libraries/SDM_Energy_Meter

COPY . /mqttclient
WORKDIR "/mqttclient"

ENTRYPOINT make

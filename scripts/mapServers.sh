#!/bin/sh

#bigserver1 
#bigserver2
servers=(
server1
server2
server3
server4
server5
server6
server7
server8
server9
server10
server11
server12
server13
server14
server15
)

for server in "${servers[@]}"; do
    echo $server > settings/mapPullTileList.ini
    echo $server
    #./Map.exe
done

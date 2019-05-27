#!/bin/sh

for zoom in $(seq 1 24); do
    echo $zoom > settings/mapPullZoom.ini
    ./Map.exe
done

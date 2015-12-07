export LD_LIBRARY_PATH=/home/jianc/Downloads/Mesa-10.1.3/lib
g++ main.cpp CanvasX11.cpp -g -lpthread -lGL -lEGL -lX11 -Iinclude

g++ texture2d.cpp CanvasX11.cpp esShader.cpp -g -lpthread -lGL -lEGL -lX11 -fpermissive -Iinclude

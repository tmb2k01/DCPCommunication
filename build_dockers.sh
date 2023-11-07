docker ps -a | grep -q slave_one_container && docker rm slave_one_container
docker ps -a | grep -q slave_two_container && docker rm slave_two_container

docker images -q slave_one_image | grep -q . && docker rmi slave_one_image
docker images -q slave_two_image | grep -q . && docker rmi slave_two_image

docker build -t slave_one_image -f Dockerfile.slave_one .
docker build -t slave_two_image -f Dockerfile.slave_two .

#docker run -it --name slave_one_container -p 8081:8081/udp -p 8081:8081/tcp slave_one_image /bin/sh
#docker run -it --name slave_two_container -p 8082:8082/udp -p 8082:8082/tcp slave_two_image /bin/sh

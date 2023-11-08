docker ps -a | grep -q master_container && docker rm master_container
docker ps -a | grep -q slave_one_container && docker rm slave_one_container
docker ps -a | grep -q slave_two_container && docker rm slave_two_container

docker images -q master_image | grep -q . && docker rmi master_image
docker images -q slave_one_image | grep -q . && docker rmi slave_one_image
docker images -q slave_two_image | grep -q . && docker rmi slave_two_image

docker build -t master_image -f Dockerfile.dcp_master .
docker build -t slave_one_image -f Dockerfile.slave_one .
docker build -t slave_two_image -f Dockerfile.slave_two .

docker network ls | grep -qw dcp-network || docker network create dcp-network

#docker run -it --name master_container --network dcp-network master_image
#docker run -it --name slave_one_container --network dcp-network slave_one_image
#docker run -it --name slave_two_container --network dcp-network slave_two_image

#docker network inspect dcp-network

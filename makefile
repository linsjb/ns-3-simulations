NAME=yakout/docker-ns3

build: Dockerfile
	docker build -t $(NAME) .

run:
	docker run --rm -it -v `pwd`/src/:/usr/ns-allinone-3.30/ns-3.30/scratch -v `pwd`/logs/:/usr/ns-allinone-3.30/ns-3.30/logs -e DISPLAY=docker.for.mac.host.internal:0 $(NAME)

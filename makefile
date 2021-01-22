NAME=ns3


build: Dockerfile
	docker build -t $(NAME) .

run:
	docker run --rm -it -v $(PWD)/src:/usr/ns-allinone-3.32/ns-3.32/scratch $(NAME)

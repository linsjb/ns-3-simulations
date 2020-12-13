NAME=gomboin99/ns-3.30


build: Dockerfile
	docker build -t $(NAME) .

run:
	docker run --rm -it -v `pwd`:/src $(NAME)

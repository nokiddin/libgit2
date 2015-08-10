
OPTIONS=-DTHREADSAFE=ON -DGIT_SSL=ON -DUSE_SSH=OFF -DBUILD_CLAR=OFF

# based on https://libgit2.github.com/docs/guides/build-and-link/
_build:
	mkdir -p build;
	cd build; cmake ${OPTIONS} ..; cmake --build .


install:
	cd build; cmake --build . --target install;


test:
	cmake -V 

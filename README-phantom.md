# Instructions

Instructions to use the fork of [google's fully-homomorphic-encryption
compiler](https://github.com/google/fully-homomorphic-encryption) found at
https://github.com/Janmajayamall/fully-homomorphic-encryption to compile a
C/C++ program into an FHE circuit for the [phantom-zone
library](https://github.com/gausslabs/phantom-zone).

You'll need `docker` installed.

Firs clone this repository and cd into it, then checkout the `phantom` branch:
```
git clone https://github.com/ed255/fully-homomorphic-encryption
cd fully-homomorphic-encryption
git checkout phantom
```

Next pick a version and pull the image.  You can see the available version in
[docker
hub](https://hub.docker.com/repository/docker/ed255/phantom-zone/general).  Each version is taged with a number and with the commit hash corresponding to https://github.com/Janmajayamall/fully-homomorphic-encryption

```
sudo docker pull ed255/phantom-zone:v2
```

Now you need to create a project folder for your C program.  You can see an
example at `projects/demo`.  The project needs to be placed in the `projects`
folder.  You can make a copy of the `demo` folder to get started, afterwards
you'll need to rename the files to use your project name as well as change the
files to use your project name:

Linux version:
```
project=myproject
cp -r projects/demo projects/${project}
cd projects/${project}
rm -rf out
mv demo.cc ${project}.cc
mv demo.h ${project}.h
mv demo_rs_lib.rs ${project}_rs_lib.rs
sed -i'' "s/demo/${project}/g" *
```

Once your code is placed in `projects/${project}/${project}.cc` and
`projects/${project}/${project}.h` you can compile it to a circuit with docker
using the following script from the root of the git repository:
```
SUDO=1 ./run-docker.sh $project
```

Once the compilation succeeds, you'll find the rust code of the circuit at `projects/${project}/out/${project}_rs_fhe_lib.rs`



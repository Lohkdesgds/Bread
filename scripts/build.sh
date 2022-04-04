echo "Building..."

cd ..
mkdir build
cd ./build

echo "Preparing to build..."

cmake ..

echo "Making build..."

make -j4

echo "The end!"

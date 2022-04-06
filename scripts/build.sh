echo "Building..."

cd ..
mkdir build
cd ./build

echo "Preparing to build..."

cmake ..

echo "Making build..."

make -j$(nproc)

cp -T ./discord-bot ./release-bot

echo "The end!"

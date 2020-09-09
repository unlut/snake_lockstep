#  store script name and number of arguments in a variable
argument_count=("$#")
script_name=("$0")
echo "Number of arguments:" ${argument_count}
echo "Script name:" ${script_name}

#  convert arguments to array
args=("$@")
echo "Arguments:" ${args[*]}

#  print first two arguments
echo "First argument:" ${args[0]}
echo "Second argument:" ${args[1]}


if [ "${args[0]}" = "rebuild" ]; then
    echo "Rebuilding..."

    #  remove build directory
    rm -r build
    
    #  create build directory
    mkdir build
    cd build

    #  call cmake to generate makefile
    cmake ..

    #  then make to build
    make

    #  copy game assets (images, sounds...) to build directory
    cd ..
    echo "Copying images..."
    cp -R images build/images
    echo "Coyping sounds..."
    cp -R sounds build/sounds
    echo "Copying fonts..."
    cp -R fonts build/fonts
    echo "Copying configs..."
    cp -R config build/config
fi




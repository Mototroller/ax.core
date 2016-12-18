#!groovy

stage 'Build'

node('master') {
    checkout scm
    sh 'rm -rf build; mkdir build; cd build && cmake .. && make'
    sh 'echo "FINISH"'
}

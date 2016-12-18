#!groovy

stage 'Build'

node('master') {
    checkout scm
    sh 'mkdir build; cd build && cmake .. && make'
    sh 'echo "FINISH"'
}

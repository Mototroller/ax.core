#!groovy

node('unix && 64bit') {
    checkout scm
    sh 'mkdir build; cd build'
    sh 'cmake .. && make'
}

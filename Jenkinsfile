#!groovy

stage 'Build'

node('master') {
    checkout scm
    sh 'mkdir build; cd build'
    sh 'cmake .. && make'
}

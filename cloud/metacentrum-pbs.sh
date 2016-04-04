#!/bin/bash
#PBS -N node-benchmarks
#PBS -l mem=2gb
#PBS -l scratch=1gb
#PBS -l nodes=1:ppn=1
#PBS -l walltime=00:04:30
#PBS -j oe
#PBS -m abe
#

# set modules
module purge
module add /software/modules/current/metabase
module add cmake-2.8
module add gcc-4.8.1
module add openmpi
module add perl-5.10.1
module add boost-1.49
module add python26-modules-gcc
module add numpy-py2.6
module add python-2.7.6-gcc

# just in case pritn g++ version
g++ --version

# running test by first cloning and then installing
# -------------------------------------------------------
git clone https://github.com/x3mSpeedy/node-benchmarks.git
cd node-benchmarks
source ./configure
make all
make test results.json
# -------------------------------------------------------


# save results
cp results.json /home/jan-hybs/results/results.json

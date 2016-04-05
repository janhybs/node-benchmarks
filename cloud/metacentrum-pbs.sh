#!/bin/bash
#PBS -N node-benchmarks
#PBS -l mem=2gb
#PBS -l scratch=1gb
#PBS -l nodes=1:ppn=1
#PBS -l walltime=00:20:00
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

# just in case print g++ version
g++ --version

# running test by first cloning and then installing
# run test in SCRATCHDIR since io operations are much faster
# first we clone node-benchmarks repo and install c++ program
# via function call we execute test several times
# -------------------------------------------------------
trap 'clean_scratch' TERM EXIT
echo "SCRATCHDIR=$SCRATCHDIR"
cd $SCRATCHDIR
git clone https://github.com/x3mSpeedy/node-benchmarks.git
cd node-benchmarks
source ./configure
make all
# -------------------------------------------------------
function run_experiment {
    HOST_NAME=$(hostname)
    NOW=$(date +"%s")
    make ARGS="results.json 1" test
    cp results.json "/storage/praha1/home/jan-hybs/results/$HOST_NAME-$NOW.json"
}
# -------------------------------------------------------
for i in {1..10}
do
    echo "------ running experiment $i ------"
    echo "-----------------------------------"
    run_experiment
done
# -------------------------------------------------------
exit

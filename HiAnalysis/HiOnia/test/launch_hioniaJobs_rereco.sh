#!/bin/bash                                          
                                                     
function command(){                                  
    echo ">>>" $@
    eval $@
}

workDir=$PWD  
                                                                       
logDir=${workDir}"/BATCHJOBS/v13.h2/"
command "mkdir -p $logDir"

#castorDirIn="/castor/cern.ch/cms/store/user/tdahms/HeavyIons/Onia/Data2010/v13/Skims/ReReco/"
castorDirOut="/castor/cern.ch/cms/store/user/tdahms/HeavyIons/Onia/Data2010/v13/Histos/ReReco/h2/"
command "rfmkdir -p  $castorDirOut"

#inputfiles=""
j=0
jobNb=""

#for files in `nsls $castorDirIn | grep onia2MuMuPAT`
for fileList in `ls inputOniaSkim_???`
do
    echo $fileList;
    #inputfiles="inputFiles=rfio:"$castorDirIn$files;
    #echo $inputfiles;
    jobNb=${j};
    let j=${j}+1;
    name="OniaSkim_ReReco_${jobNb}"
    outfilename="Histos_${name}.root"
    secoutfilename="DataSet_${name}.root"

#Start to write the script
	cat > job_hionia_rereco_${jobNb}.sh << EOF
#!/bin/sh

function command(){                                  
     echo ">>>" \$@                                   
     eval \$@                                         
}

workDir=$PWD  
logDir="${logDir}"
castorDirOut="${castorDirOut}"


cd /afs/cern.ch/user/t/tdahms/scratch0/HeavyIons2010/CMSSW_3_9_9_patch1/src/
eval \`scramv1 runtime -sh\`
cd -
cp /afs/cern.ch/user/t/tdahms/scratch0/HeavyIons2010/CMSSW_3_9_9_patch1/src/HiAnalysis/HiOnia/test/hioniaanalyzer_cfg.py .
cp /afs/cern.ch/user/t/tdahms/scratch0/HeavyIons2010/CMSSW_3_9_9_patch1/src/HiAnalysis/HiOnia/test/${fileList} .
echo "before running cmsRun"
ls -l

command "cmsRun hioniaanalyzer_cfg.py inputFiles_clear inputFiles_load=${fileList} outputFile=${outfilename} secondaryOutputFile=${secoutfilename} print 2>&1 | tee -a $logDir${name}.log"
echo "after running cmsRun"
ls -l
command "rfcp ${outfilename} $castorDirOut"
command "gzip -f $logDir${name}.log"
rm -f hioniaanalyzerpat_cfg.py

EOF

	chmod 755 job_hionia_rereco_${jobNb}.sh
	
	bsub -q 1nd -J $name /afs/cern.ch/user/t/tdahms/scratch0/HeavyIons2010/CMSSW_3_9_9_patch1/src/HiAnalysis/HiOnia/test/job_hionia_rereco_${jobNb}.sh


done





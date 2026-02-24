#!/bin/bash

backup_one="/mnt/FASTstorage/FASTprogs"
backup_two="/mnt/REBORN/FASTERprogs"
backup_three="/mnt/SUPER_CAVALEIRO/progsBackup"

pushd "${backup_one}/emotionstreamer/code" &&
bash makeAll.sh &&
popd &&
bash updateRepoMasterBranch.sh &&
rm -rf "${backup_one}/emotionstreamer_pub/code" &&
rm -rf "${backup_one}/emotionstreamer_priv/code" &&
rm -rf "${backup_two}/emotionstreamer_pub/code" &&
rm -rf "${backup_two}/emotionstreamer_priv/code" &&
rm -rf "${backup_two}/emotionstreamer/code" &&
rm -rf "${backup_three}/emotionstreamer_pub/code" &&
rm -rf "${backup_three}/emotionstreamer_priv/code" &&
rm -rf "${backup_three}/emotionstreamer/code" &&
cp -rf *.sh "${backup_one}/emotionstreamer_pub" &&
cp -rf code "${backup_one}/emotionstreamer_pub" &&
cp -rf *.sh "${backup_one}/emotionstreamer_priv" &&
cp -rf code "${backup_one}/emotionstreamer_priv" &&
cp -rf *.sh "${backup_two}/emotionstreamer_pub" &&
cp -rf code "${backup_two}/emotionstreamer_pub" &&
cp -rf *.sh "${backup_two}/emotionstreamer_priv" &&
cp -rf code "${backup_two}/emotionstreamer_priv" &&
cp -rf *.sh "${backup_two}/emotionstreamer" &&
cp -rf code "${backup_two}/emotionstreamer" &&
cp -rf *.sh "${backup_three}/emotionstreamer_pub" &&
cp -rf code "${backup_three}/emotionstreamer_pub" &&
cp -rf *.sh "${backup_three}/emotionstreamer_priv" &&
cp -rf code "${backup_three}/emotionstreamer_priv" &&
cp -rf *.sh "${backup_three}/emotionstreamer" &&
cp -rf code "${backup_three}/emotionstreamer" &&
pushd "${backup_one}/emotionstreamer_pub" &&
bash updateRepoMasterBranch.sh &&
popd

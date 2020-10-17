scriptpath=$(realpath $0) # This get the script path + the name, e.g /path/to/script/scriptname.sh
scriptdir=$(dirname $scriptpath) # This get the script directory e.g /path/to/script which is what I need

source $scriptdir/backup.conf

backup_conf="$scriptdir/backup.conf"

backup_edit_retention()
{
  read -p  $'\t\t\t\t'"How Many Daily Backup Do You Want To Keep (Default To 7 If You Skip): " dailybackup
      #
      #   If User Gives an Empty Answer, we Default it to 7 otherwise, we store the user input
      #
      if [[ "$dailybackup" = "" ]];then
            dailybackup=7
            sed -i "s/dailybackup.*/dailybackup=$dailybackup/" $backup_conf # Edit the default daily value to the retention config
      else 
            sed -i "s/dailybackup.*/dailybackup=$dailybackup/" $backup_conf # Edit the User daily value to the retention config
      fi

  read -p  $'\t\t\t\t'"How Many Weekly Backup Do You Want To Keep (Default To 4 If You Skip): " weeklybackup
      #
      #   If User Gives an Empty Answer, we Default it to 4 otherwise, we store the user input
      #
      if [[ "$weeklybackup" = "" ]];then
            weeklybackup=7
            sed -i "s/weeklybackup.*/weeklybackup=$weeklybackup/" $backup_conf # Edit the default weekly value to the retention config
      else
            sed -i "s/weeklybackup.*/weeklybackup=$weeklybackup/" $backup_conf # Edit the User weekly value to the retention config
      fi

  read -p  $'\t\t\t\t'"How Many Monthly Backup Do You Want To Keep (Default To 12 If You Skip): " monthlybackup
      #
      #   If User Gives an Empty Answer, we Default it to 12 otherwise, we store the user input
      #
      if [[ "$monthlybackup" = "" ]];then
            monthlybackup=12
            sed -i "s/monthlybackup.*/monthlybackup=$monthlybackup/" $backup_conf # Edit the default monthly value to the retention config
      else
            sed -i "s/monthlybackup.*/monthlybackup=$monthlybackup/" $backup_conf # Edit the User monthly value to the retention config
      fi
      echo
      echo -e "\t\t\t\tBackup Retention Successfully Edited"
      echo
} # END backup_edit_retention

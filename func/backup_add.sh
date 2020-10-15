#
# Includes Backup Retention Configuration
#
scriptpath=`realpath $0` # This get the script path + the name, e.g /path/to/script/scriptname.sh
scriptdir=`dirname $scriptpath` # This get the script directory e.g /path/to/script which is what I need

source $scriptdir/backup.conf

backup_conf="$scriptdir/backup.conf"

backup_add() {
  #
  #   Time For Backing Up
  # 

  date=`date +"%Y-%m-%d"`
  now=`date +"%c"`

  echo -e "\t\t\t\tGetting Started..."

  #
  #   If no backup folder, it means the user hasn't created any backup, so, we collect the neccessary info and
  #   pass it into the backup.conf
  #
  if [ ! -d /backup ]; then

      read -p  $'\t\t\t\t'"How Many Daily Backup Do You Want To Keep (Default To 7 If You Skip): " dailybackup
          #
          #   If User Gives an Empty Answer, we Default it to 7 otherwise, we store the user input
          #
          if [[ "$dailybackup" = "" ]];then
              dailybackup=7
              sed -i "s/dailybackup.*/dailybackup=$dailybackup/" $backup_conf # Pass the default daily value to the retention config
          else 
              sed -i "s/dailybackup.*/dailybackup=$dailybackup/" $backup_conf # Pass the user daily value to the retention config
          fi

      read -p  $'\t\t\t\t'"How Many Weekly Backup Do You Want To Keep (Default To 4 If You Skip): " weeklybackup
          #
          #   If User Gives an Empty Answer, we Default it to 4 otherwise, we store the user input
          #
          if [[ "$weeklybackup" = "" ]];then
              weeklybackup=7
              sed -i "s/weeklybackup.*/weeklybackup=$weeklybackup/" $backup_conf # Pass the default weekly value to the retention config
          else
              sed -i "s/weeklybackup.*/weeklybackup=$weeklybackup/" $backup_conf # Pass the user weekly value to the retention config
          fi

      read -p  $'\t\t\t\t'"How Many Monthly Backup Do You Want To Keep (Default To 12 If You Skip): " monthlybackup
          #
          #   If User Gives an Empty Answer, we Default it to 12 otherwise, we store the user input
          #
          if [[ "$monthlybackup" = "" ]];then
              monthlybackup=12
              sed -i "s/monthlybackup.*/monthlybackup=$monthlybackup/" $backup_conf # Pass the default monthly value to the retention config
          else
               sed -i "s/monthlybackup.*/monthlybackup=$monthlybackup/" $backup_conf # Pass the user monthly value to the retention config
          fi

          
  fi # END [ ! -d /backup ]

  # year=`date +"%Y"` #   Store Current year

  # Store PAM Accounts For Use In Bind or SFTP Backup
  pamaccounts="/etc/passwd /etc/shadow /etc/group"

  echo -e "\t\t\t\tInitiating - $now" | tee -a backup_log/$date.log #stdout -> log & stdout
  echo -e "\t\t\t\tProgressing - $now" | tee -a backup_log/$date.log
  echo

  #
  #   This command initializes an empty repository.
  #   A repository is a filesystem directory containing the deduplicated data from zero or more archives.
  #

  #
  #   While Using encryption would be secure, I have default to not using it for a couple of reason.
  #   If an Attacker could break into your system, then they can access your data anyway
  #

  # borg init --encryption=none $backupfolder 

  #
  #   Checking Bind9 Availability
  #

  echo -e "\t\t\t\tBacking up DNS Data if you have that created..."

  if command -v named 2>> backup_log/$date.log &>/dev/null
  then
      echo

      echo -e "\t\t\t\tBind9 Okay...."

        if [ ! -d /etc/bind ];then
          echo -e "\t\t\t\tCouldn't Find Any DNS configurations, this is fine if this server isn't managing your DNS...Moving on..."

        else

        # Create Bind Backup Directories If it is empty, this only happens once
        if [[ "$backupbind" = "" ]]; then
          backupbind=/backup/borg/bind
          sed -i "s/backupbind.*/backupbind=\/backup\/borg\/bind/" $backup_conf
          mkdir -p $backupbind
          echo -e "\t\t\t\tCreating Bind Repo - $now" | tee -a backup_log/$date.log
          borg init --encryption=none $backupbind
        fi

           echo -e "\t\t\t\tBacking Up Bind Data - $now" | tee -a backup_log/$date.log

           bind=/etc/bind # If there is bind directory, we store it for compression

           # This store all the file into the bkdate, which would be inside bind_todaysdate
          borg create                       \
            --verbose                       \
            --filter AME                    \
            --list                          \
            --stats                         \
            --show-rc                       \
            --compression lz4               \
                                            \
            $backupbind::$date "$bind" $pamaccounts 2>> backup_log/$date.log >/dev/null &
            echo
            echo -e "\t\t\t\tBind Backed Up - $now" | tee -a backup_log/$date.log

            #
            #  Prune options decide what particular archives from your Borg backup get deleted over time
            #
            #  The below Keeps Seven Daily Backups, Four Weekly Backups, and Twelve Monthly Backups
            #  This is kinda confusing, but this is how it works, say you only have --keep-daily 7, it will keep the latest 
            #  backup on each of the most recent 7 days which have backups. 
            #  This means it would keep the last daily backup (for up to 7 days).
            #
            #  In short, if you have Day 1 2 3 4 5 6 7, if day 8 comes, it would delete day 1 and replace the data with day 8
            #  So, day 8 would be the new day 1, this is what it means by keep-daily for 7 days
            #

          echo -e "\t\t\t\tPruning Bind data - $now" | tee -a backup_log/$date.log
          borg prune -v --list --keep-daily=$dailybackup --keep-weekly=$weeklybackup --keep-monthly=$monthlybackup $backupbind 2>> backup_log/$date.log >/dev/null &


          
        fi # END [ ! -d /etc/bind ]

  else

      echo -e "\t\t\t\tYou don't have bind installed nor do you have DNS created...this is fine if this server isn't managing your DNS...Moving on..."
  fi # END command -v named 2>> backup_log/$date.log &>/dev/null

  #
  #   Checking mariadb Availability
  #
  if command -v mariadb 2>> backup_log/$date.log &>/dev/null
  then
      echo
      echo -e "\t\t\t\tChecking mariadb Availability...."
      echo -e "\t\t\t\tmariadb okay...."
      echo -e "\t\t\t\tBacking Up All mariadb database - $now" | tee -a backup_log/$date.log


      echo -e "\t\t\t\tEnter Mysql root Password: \c"
      read -s mysqlpass

      # Until The Password is Correct, Keep Looping
      until mysql -u root -p$mysqlpass  -e ";" ; do
       read -s -p  $'\t\t\t\t'"Incorrect Password, Please Retry: " mysqlpass
      done

      echo -e "\t\t\t\tPassword Correct: \c" 

      mysqldump --user=root --password=$mysqlpass --lock-tables --all-databases > dbs.sql

      echo >> $backup_conf
      echo "mysqlpass=$mysqlpass" >> $backup_conf # Pass rclone path to the retention config 

        # Create Database Backup Directories If it is empty, this only happens once
        if [[ "$backupdatabase" = "" ]]; then
          backupdatabase=/backup/borg/database
          sed -i "s/backupdatabase.*/backupdatabase=\/backup\/borg\/database/" $backup_conf
          mkdir -p $backupdatabase
          echo -e "\t\t\t\tCreating Database Repo - $now" | tee -a backup_log/$date.log
          borg init --encryption=none $backupdatabase
        fi


      # This store all the file into the bkdate, which would be inside backup_todaysdate  
       borg create                       \
        --verbose                       \
        --filter AME                    \
        --list                          \
        --stats                         \
        --show-rc                       \
        --compression lz4               \
                                        \
        $backupdatabase::$date dbs.sql  2>> backup_log/$date.log >/dev/null &
        echo
        echo -e "\t\t\t\tDatabase Backed Up - $now" | tee -a backup_log/$date.log

          echo -e "\t\t\t\tPruning Full database data - $now" | tee -a backup_log/$date.log
          borg prune -v --list --keep-daily=$dailybackup --keep-weekly=$weeklybackup --keep-monthly=$monthlybackup $backupdatabase 2>> backup_log/$date.log >/dev/null &

          rm dbs.sql

  else

      echo -e "\t\t\t\tYou don't have mariadb installed nor do you have any db created...."
      echo -e "\t\t\t\treturning to main menu...."
      return 1
  fi # END command -v mariadb 2>> backup_log/$date.log &>/dev/null

  #
  #   Checking if SFTP Users are created
  # 

  echo -e "\t\t\t\tChecking SFTP Users Availability...."

  if [ ! -d /sftpusers/jailed ];then
     echo -e "\t\t\t\tCouldn't Find Any SFTP Users, this is fine if you haven't created an SFTP User"
  else

     echo -e "\t\t\t\t SFTP Users Okay...."
     sftp=/sftpusers/jailed # If there is sftpusers directory, we store it for compression later

        # Create SFTP Backup Directories If it is empty, this only happens once
        if [[ "$backupsftp" = "" ]]; then
          backupsftp=/backup/borg/sftp
          sed -i "s/backupsftp.*/backupsftp=\/backup\/borg\/sftp/" $backup_conf
          mkdir -p $backupsftp
          echo -e "\t\t\t\tCreating SFTP Repo - $now" | tee -a backup_log/$date.log
          borg init --encryption=none $backupsftp
        fi

      echo -e "\t\t\t\tBacking Up All SFTP Users - $now" | tee -a backup_log/$date.log

      # This store all the file into the bkdate, which would be inside backup_todaysdate
        borg create                       \
          --verbose                       \
          --filter AME                    \
          --list                          \
          --stats                         \
          --show-rc                       \
          --compression lz4               \
                                          \
          $backupsftp::$date "$sftp" $pamaccounts 2>> backup_log/$date.log >/dev/null &
          echo
          echo -e "\t\t\t\tSFTP Backed Up - $now" | tee -a backup_log/$date.log


          echo -e "\t\t\t\tPruning SFTP Users Data - $now" | tee -a backup_log/$date.log
          borg prune -v --list --keep-daily=$dailybackup --keep-weekly=$weeklybackup --keep-monthly=$monthlybackup $backupsftp 2>> backup_log/$date.log >/dev/null &
          
  fi # END [ ! -d /sftpusers/jailed ];

  #
  #   Checking if letsencrypt certificate is available
  # 

  echo -e "\t\t\t\tChecking letsencrypt certificate...."

  if [ ! -d /etc/letsencrypt ];then
     echo -e "\t\t\t\tCouldn't Find Any letsencrypt certificate, this is fine if you haven't created one...Moving on..."
  else

     letsencrypt=/etc/letsencrypt # If there is letsencrypt directory, we store it for compression later


        # Create Certificate Backup Directories If it is empty, this only happens once
        if [[ "$backupletsencrypt" = "" ]]; then
          backupletsencrypt=/backup/borg/letsencrypt
          sed -i "s/backupletsencrypt.*/backupletsencrypt=\/backup\/borg\/letsencrypt/" $backup_conf
          mkdir -p $backupletsencrypt
          echo -e "\t\t\t\tCreating Cert Repo - $now" | tee -a backup_log/$date.log
          borg init --encryption=none $backupletsencrypt
        fi

     borg create                       \
        --verbose                       \
        --filter AME                    \
        --list                          \
        --stats                         \
        --show-rc                       \
        --compression lz4               \
                                        \
        $backupletsencrypt::$date "$letsencrypt"  2>> backup_log/$date.log >/dev/null &
        echo
        echo -e "\t\t\t\tLetsencrypt Backed Up - $now" | tee -a backup_log/$date.log

         echo -e "\t\t\t\tPruning Letsencrypt Cert Data - $now" | tee -a backup_log/$date.log
         borg prune -v --list --keep-daily=$dailybackup --keep-weekly=$weeklybackup --keep-monthly=$monthlybackup $backupletsencrypt 2>> backup_log/$date.log >/dev/null &
          
  fi # END [ ! -d /etc/letsencrypt ]


  #
  #   Checking Nginx and Vhosts Availability
  #

  echo -e "\t\t\t\tChecking Nginx Config and Vhost..."

  if command -v nginx 2>> backup_log/$date.log &>/dev/null
  then
      echo
      echo -e "\t\t\t\tNginx okay...."

      if [[ ! -d /etc/nginx && ! -d /var/www ]]
      then   
        echo -e "\t\t\t\tCouldn't Find Any Nginx configurations nor website hosts...."
        return 1

      else

        # Create Site Backup Directories If it is empty, this only happens once
        if [[ "$backupsitedir" = "" ]]; then
          backupsitedir=/backup/borg/sitedir
          sed -i "s/backupsitedir.*/backupsitedir=\/backup\/borg\/sitedir/" $backup_conf
          mkdir -p $backupsitedir
          echo -e "\t\t\t\tCreating Site Dir Repo - $now" | tee -a backup_log/$date.log
          borg init --encryption=none $backupsitedir
        fi

        nginxconfig=/etc/nginx  # If there is an nginx directory, we store it for compression later
        vhosts=/var/www         # If there is a /var/www directory, we store it for compression later

        echo -e "\t\t\t\tBacking Up Nginx Config and All Sites Vhosts Data - $now" | tee -a backup_log/$date.log

        # This store all the file into the bkdate, which would be inside nginx_todaysdate
        # This store all the website vhost; /var/www

        borg create                       \
          --verbose                       \
          --filter AME                    \
          --list                          \
          --stats                         \
          --show-rc                       \
          --compression lz4               \
                                          \
          $backupsitedir::$date "$vhosts" "$nginxconfig"  2>> backup_log/$date.log >/dev/null &
          echo
          echo -e "\t\t\t\tSite Dir Backed Up - $now" | tee -a backup_log/$date.log

          echo -e "\t\t\t\tPruning Nginx and vhost data - $now" | tee -a backup_log/$date.log
          borg prune -v --list --keep-daily=$dailybackup --keep-weekly=$weeklybackup --keep-monthly=$monthlybackup $backupsitedir 2>> backup_log/$date.log >/dev/null &

      fi # END [[ ! -d /etc/nginx && ! -d /var/www ]]

  else
      echo -e "\t\t\t\tYou don't have any Nginx config created...."
      echo -e "\t\t\t\tquitting...."
      return 1
  fi # END command -v nginx 2>> backup_log/$date.log &>/dev/null

  #
  #   Backing up the whole bind folder store in the variable
  #

# Multiple Line Comment For Further Use Cases in The Future

echo "
  borg create                       \
    --verbose                       \
    --filter AME                    \
    --list                          \
    --stats                         \
    --show-rc                       \
    --compression lz4               \
                                    \
    $backupfolder::$bkdate          \ # This store all the file into the bkdate, which would be inside backup_todaysdate
    $bind                           \ # This store all the /etc/bind data if one is available
    $vhosts                         \ # This store all the website vhost; /var/www
    $nginxconfig                    \ # This store all the nginx config; /etc/nginx
    $sftp                           \
    $letsencrypt  2>> backup_log/$date.log >/dev/null &

" &>/dev/null

#   Setting Cron Permission

chmod a+x $scriptdir/backup_cron.sh

#
# Checking If Rclone Path is Set, if Yes, we sync to rclone path
#

if command -v rclone 2>> backup_log/$date.log &>/dev/null
then 
    echo -e "\t\t\t\tRclone Okay"
else
    sudo apt-get -y install rclone
fi

#
#   Ask If User wanna sync to Remote Storage
#

if yes_no "Do you want to Sync backup to rclone Remote Cloud Storage"
then
    echo -e "\t\t\t\tYour Rclone Remote Path(Skip By Hitting Enter): \c"
    read rcloneto

    if [[ "$rcloneto" = "" ]];then
        rcloneto=
        sed -i "s/rcloneto.*/rcloneto=$rcloneto/" $backup_conf # Make rcloneto empty
    else
        sed -i "s/rcloneto.*/rcloneto=$rcloneto/" $backup_conf # Pass rclone path to the retention config
    fi
fi


if [[ -z "$rcloneto" ]]; then

    echo "Not Pushing To Cloud Storage"

else

    FROM=/backup/borg
    start=$(date +'%s')
    echo "$(date "+%Y-%m-%d %T") RCLONE Sync Started" | tee -a backup_log/$date.log
    #
    #   --transfers means the number of file transfers to run in parallel
    #
    #   --checkers controls the number of directories being listed simultaneously. Rclone also uses --checkers as a 
    #   general measure of how parallel should this operation be, so for instance when deleting files, 
    #   how many deletes to do in parallel.
    #
    #   --delete-after will delay deletion of files until all new/updated files have been successfully transferred.
    #
    #   --min-age 2h means no files younger than 2 hours will be transferred.
    #

    rclone sync "$FROM" "$rcloneto" --transfers=20 --checkers=15 --delete-after --min-age 2h --log-file=backup_log/$date.log
    echo "$(date "+%Y-%m-%d %T") RCLONE Upload Apparently Succeeded IN $(($(date +'%s') - $start)) SECONDS" | tee -a backup_log/$date.log
fi

scriptpath=`realpath $0` # This get the script path + the name, e.g /path/to/script/scriptname.sh
scriptdir=`dirname $scriptpath` # This get the script directory e.g /path/to/script which is what I need

echo "
         Note: To Periodically Run The Backup, You Should add 
         $scriptdir/backup_cron.sh to your crontab, and make sure you run
         it once a day.
         " | boxes -d columns

  } # END backup_add

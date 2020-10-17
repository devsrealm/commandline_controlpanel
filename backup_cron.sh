#!/bin/bash

#
# Backup Cron Job
#

#
# Includes Backup Retention Configuration
#
source backup.conf

backup_add() {

  #
  #   This answers the Warning: Attempting to access a previously unknown unencrypted repository
  #

  export BORG_UNKNOWN_UNENCRYPTED_REPO_ACCESS_IS_OK=yes

  #
  #   Time For Backing Up
  # 

  date=$(date +"%Y-%m-%d")
  now=$(date +"%c")

  echo -e "\t\t\t\tGetting Started..."

  # year=`date +"%Y"` #   Store Current year

  # Store PAM Accounts For Use In Bind or SFTP Backup
  pamaccounts="/etc/passwd /etc/shadow /etc/group"

  echo "Initiating - $now" | tee -a backup_log/"$date".log #stdout -> log & stdout
  echo "Progressing - $now" | tee -a backup_log/"$date".log

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

  if command -v named 2>> backup_log/"$date".log >/dev/null
  then
      echo

      echo -e "\t\t\t\tBind9 Okay...."

        if [ ! -d /etc/bind ];then
          echo -e "\t\t\t\tCouldn't Find Any DNS configurations, this is fine if this server isn't managing your DNS...Moving on..."

        else

        # Resetting Bind Backup Directory If it is empty, this only happens once
        if [[ "$backupbind" = "" ]]; then
          backupbind=/backup/borg/bind
          echo -e "\t\t\t\tResetting Bind Backup Directory - $now"
        fi

          bind=/etc/bind # If there is bind directory, we store it for compression later

           echo -e "\t\t\t\tBacking Up Bind Data - $now" | tee -a backup_log/"$date".log

          borg create                       \
            --verbose                       \
            --filter AME                    \
            --list                          \
            --stats                         \
            --show-rc                       \
            --compression lz4               \
                                            \
            $backupbind::"$date" "$bind" $pamaccounts 2>> backup_log/"$date".log >/dev/null &

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

          echo -e "\t\t\t\tPruning Bind data - $now" | tee -a backup_log/"$date".log
          borg prune -v --list --keep-daily="$dailybackup" --keep-weekly="$weeklybackup" --keep-monthly="$monthlybackup" $backupbind 2>> backup_log/"$date".log >/dev/null &


          
        fi # END [ ! -d /etc/bind ]

  else

      echo -e "\t\t\t\tYou don't have bind installed nor do you have DNS created...this is fine if this server isn't managing your DNS...Moving on..."
  fi # END command -v named 2>> backup_log/$date.log &>/dev/null

  #
  #   Checking mariadb Availability
  #
  if command -v mariadb 2>> backup_log/"$date".log >/dev/null
  then
      echo
      echo -e "\t\t\t\tChecking mariadb Availability...."
      echo -e "\t\t\t\mariadb okay...."
      echo -e "\t\t\t\tBacking Up All mariadb database - $now" | tee -a backup_log/"$date".log


      sudo mysqldump --user=root --password="$mysqlpass" --lock-tables --all-databases > dbs.sql
        # Resetting Database Backup Directory If it is empty, this only happens once
        if [[ "$backupdatabase" = "" ]]; then
          backupdatabase=/backup/borg/database
          echo -e "\t\t\t\tResetting Database Backup Directory - $now" | tee -a backup_log/"$date".log
        fi


       borg create                       \
        --verbose                       \
        --filter AME                    \
        --list                          \
        --stats                         \
        --show-rc                       \
        --compression lz4               \
                                        \
        $backupdatabase::"$date" dbs.sql  2>> backup_log/"$date".log >/dev/null &

          echo "\t\t\t\tPruning Full database data - $now" | tee -a backup_log/"$date".log
          borg prune -v --list --keep-daily="$dailybackup" --keep-weekly="$weeklybackup" --keep-monthly="$monthlybackup" $backupdatabase 2>> backup_log/"$date".log >/dev/null &

        rm dbs.sql

  else

      echo -e "\t\t\t\tYou don't have mariadb installed nor do you have any db created...."
      echo -e "\t\t\t\treturning to main menu...."
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

        # Resetting SFTP Backup Directory If it is empty, this only happens once
        if [[ "$backupsftp" = "" ]]; then
          backupsftp=/backup/borg/sftp
          echo -e "\t\t\t\tResetting SFTP Backup Directory - $now" | tee -a backup_log/"$date".log
        fi

      echo -e "\t\t\t\tBacking Up All SFTP Users - $now" | tee -a backup_log/"$date".log

        borg create                       \
          --verbose                       \
          --filter AME                    \
          --list                          \
          --stats                         \
          --show-rc                       \
          --compression lz4               \
                                          \
          $backupsftp::"$date" "$sftp" $pamaccounts 2>> backup_log/"$date".log >/dev/null &


          echo "\t\t\t\tPruning SFTP Users Data - $now" | tee -a backup_log/"$date".log
          borg prune -v --list --keep-daily="$dailybackup" --keep-weekly="$weeklybackup" --keep-monthly="$monthlybackup" $backupsftp 2>> backup_log/"$date".log >/dev/null &
          
  fi # END [ ! -d /sftpusers/jailed ];

  #
  #   Checking if letsencrypt certificate is available
  # 

  echo -e "\t\t\t\tChecking letsencrypt certificate...."

  if [ ! -d /etc/letsencrypt ];then

     echo -e "\t\t\t\tCouldn't Find Any letsencrypt certificate, this is fine if you haven't created one...Moving on..."
  else

     letsencrypt=/etc/letsencrypt # If there is letsencrypt directory, we store it for compression later


        # Resetting Certificate Backup Directory If it is empty, this only happens once
        if [[ "$backupletsencrypt" = "" ]]; then
          backupletsencrypt=/backup/borg/letsencrypt
          echo -e "\t\t\t\tResetting Certificate Backup Directory - $now" | tee -a backup_log/"$date".log
        fi

     borg create                       \
        --verbose                       \
        --filter AME                    \
        --list                          \
        --stats                         \
        --show-rc                       \
        --compression lz4               \
                                        \
        $backupletsencrypt::"$date" "$letsencrypt"  2>> backup_log/"$date".log >/dev/null &

         echo "\t\t\t\tPruning Letsencrypt Cert Data - $now" | tee -a backup_log/"$date".log
         borg prune -v --list --keep-daily="$dailybackup" --keep-weekly="$weeklybackup" --keep-monthly="$monthlybackup" $backupletsencrypt 2>> backup_log/"$date".log >/dev/null &
          
  fi # END [ ! -d /etc/letsencrypt ]


  #
  #   Checking Nginx and Vhosts Availability
  #

  echo -e "\t\t\t\tChecking Nginx Config and Vhost..."

  if command -v nginx 2>> backup_log/"$date".log >/dev/null
  then
      echo
      echo -e "\t\t\t\tNginx okay...."

      if [[ ! -d /etc/nginx && ! -d /var/www ]]
      then   
        echo -e "\t\t\t\tCouldn't Find Any Nginx configurations nor website hosts...."
      else

        # Resetting Site Backup Directory If it is empty, this only happens once
        if [[ "$backupsitedir" = "" ]]; then
          backupsitedir=/backup/borg/sitedir
          echo -e "\t\t\t\tResetting Site Backup Directory - $now" | tee -a backup_log/"$date".log
        fi

        nginxconfig=/etc/nginx  # If there is an nginx directory, we store it for compression later
        vhosts=/var/www         # If there is a /var/www directory, we store it for compression later

        echo -e "\t\t\t\tBacking Up Nginx Config and All Sites Vhosts Data - $now" | tee -a backup_log/"$date".log

        borg create                       \
          --verbose                       \
          --filter AME                    \
          --list                          \
          --stats                         \
          --show-rc                       \
          --compression lz4               \
                                          \
          $backupsitedir::"$date" "$vhosts" "$nginxconfig"  2>> backup_log/"$date".log >/dev/null &

          echo "\t\t\t\tPruning Nginx and vhost data - $now" | tee -a backup_log/"$date".log
          borg prune -v --list --keep-daily="$dailybackup" --keep-weekly="$weeklybackup" --keep-monthly="$monthlybackup" $backupsitedir 2>> backup_log/"$date".log >/dev/null &

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

#
# Checking If Rclone Path is Set, if Yes, we sync to rclone path
#
if [[ -z "$rcloneto" ]]; then

    echo -e "/t/t/t/tNot Pushing To Cloud Storage"

else

    FROM=/backup/borg
    start=$(date +'%s')
    echo "$(date "+%Y-%m-%d %T") RCLONE Sync Started" | tee -a backup_log/"$date".log
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

    rclone sync "$FROM" "$rcloneto" --transfers=20 --checkers=15 --delete-after --min-age 2h --log-file=backup_log/"$date".log
    echo "$(date "+%Y-%m-%d %T") RCLONE Upload Apparently Succeeded IN $(($(date +'%s') - $start)) SECONDS" | tee -a backup_log/"$date".log
fi

} # END backup_add

backup_add

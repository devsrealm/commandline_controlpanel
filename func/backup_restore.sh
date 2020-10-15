# Create Restore Log
mkdir -p restore_log # Create Backup Directory If It Already Doesn't Exist

restorechecker() {

    restorestat=$1

    if [[ $restorestat != 0 ]];then
    echo "Restoration Error, Check restore_log/$date.log"
    exit 1
    fi

}

backup_restore()
{
 if [ ! -d /backup ]; then
 echo -e "\t\t\t\tYou don't have a backup...exiting"
 return 1
 fi

 if yes_no "Do You Want to Read a Tip Before Restoring (Recommended If This is Your First Time)"
 then

  echo "
        Make sure you have correctly set your locale before restoring anything, this ensure the 
        restoration restores the correct locale of your language when restoring, 
        e.g, if your files contain a certain character that is known to only your language, 
        e.g French, then you might want to set your locale to a French language.

        To do this, exit the program and run \"sudo dpkg-reconfigure locales\" (without the quote), 
        Then, you pick a UTF-8 locale in your language to ensure compatibility with software. 
        Mine is Nigeria, so, I picked \"en_NG UFT-8\" use spacebar to select it, and then hit Enter, 
        you then select the default locale to the same language, 
        Hit Enter Again, and this would generate the locale for you.

        Finalize, the configuration by adding \"export LANG=en_NG.UTF-8 LC_ALL=en_NG.UTF-8\" (without the quote),
        to the end of .bashrc file, once you are done, source it using \"source .bashrc\" 
        remember, this is a Nigerian Locale, so, 
        if you want US language, you would use \"export LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8\" 
        (again, without the quote)
        " | boxes -d columns
  fi

  echo -e "\t\t\t\tWhat Would You Like To Restore...\n"
    while :
          do
          #
          #   Display DNS Menu
          #
                  echo "
                        1.) Restore The Full Sites
                        2.) Restore Only DNS Configuration
                        3.) Restore Only SFTP Users
                        4.) Restore Only Let's Encrypt Certficate
                        5.) Restore Only Database File
                        6.) Exit

                        " | boxes -d columns

                        #
                #   Prompt for an answer
                #
                echo -e "\t\t\t\tAnswer (or 'q' to quit): \c?"
                read ans junk

                #
                #   Empty answers (pressing ENTER) cause the menu to redisplay,
                #   so, this goes back around the loop
                #   We only make it to the "continue" bit if the "test"
                #   program ("[") returned 0 (True)
                #
                [ "$ans" = "" ] && continue

                #
                #   Decide what to do base on user selection
                #

                    case $ans in
                    1)     restore_fullsite
                    ;;
                    2)     restore_dns
                    ;;
                    3)     restore_sftp
                    ;;
                    4)     restore_certificate
                    ;;
                    5)     restore_db
                    ;;
                    6)     quit 0
                    ;;
                    q*|Q*) quit 0
                    ;;
                    *)     echo -e "\t\t\t\tPlease Enter a Number Between 1 and 6";;
                esac
                #
                #   Pause to give the user a chance to see what's on the screen, this way, we won't lose some infos
                #
                pause

    done        


} # END backup_restore



restore_fullsite()
{
  # Call the other restore functions

  restore_dns
  restore_sftp
  restore_certificate
  restore_database

  backupsitedir=/backup/borg/sitedir

  date=`date +"%Y-%m-%d"`

  if [[ ! -d "$backupsitedir" ]]; then
      echo -e "\t\t\t\tYou can't restore Sites! You either don't have site directories or you don't have any backed up"
      return 1
  else

  echo "
        Input The Date You Want To Restore, e.g, if you want to restore the backup for today ($date),
        you type $date and hit enter
        " | boxes -d columns

  echo -e "\t\t\t\tThis is the back up retained for your sites"
  echo
  borg list $backupsitedir  
  echo

  read -p  $'\t\t\t\t'"Which Backup Do You Wish To Restore " sitedirrestorechoice

  while [[ $sitedirrestorechoice = "" ]]; do
      echo -e "\t\t\t\tPlease Enter The Backup You Wish To Restore: \c"
      read sitedirrestorechoice
  done


  #
  # I'll need to get the full path of the script directory cos I am about to CD for the restore operation
  #

  scriptpath=`realpath $0` # This get the script path + the name, e.g /path/to/script/scriptname.sh
  scriptdir=`dirname $scriptpath` # This get the script directory e.g /path/to/script which is what I need

  echo -e "\t\t\t\tCreating a Temp Directory To Extract The File Into"
  TMPDIR=`mktemp -d /tmp/restore_sitedir.XXXXXXXXXXXXXXXX` || exit 1

  cd $TMPDIR
  borg extract --list --strip-components $backupsitedir::$sitedirrestorechoice 2>> $scriptdir/restore_log/$date.log >/dev/null &

  restorechecker $?

  echo -e "\t\t\t\tMoving Certficate Into Appropriate Folder"
  # Copy Recursively Including Hidden Files
  cp -Rf $TMPDIR/var/www/. /var/www 2>> $scriptdir/restore_log/$date.log &>/dev/null
  cp -Rf $TMPDIR/etc/. /etc 2>> $scriptdir/restore_log/$date.log &>/dev/null

  restorechecker $?

  echo -e "\t\t\t\tDone Restoring"
  echo -e "\t\t\t\tRemoving Temp Directory"

  rm -r $TMPDIR

  cd $scriptdir

  return 0
              
  fi
}

#
#   Function To DNS Files
#

restore_dns()
{
  backupbind=/backup/borg/bind

  date=`date +"%Y-%m-%d"`

  if [[ ! -d "$backupbind" ]]; then
      echo -e "\t\t\t\tYou can't restore DNS Configurations! This is fine if this server isn't managing your DNS"
  else

  echo "
        Input The Date You Want To Restore, e.g, if you want to restore the backup for today ($date),
        you type $date and hit enter
        " | boxes -d columns

  echo -e "\t\t\t\tThis is the back up retained for your DNS"
  echo
  borg list $backupbind  
  echo 

  read -p  $'\t\t\t\t'"Which Backup Do You Wish To Restore " bindrestorechoice

  while [[ $bindrestorechoice = "" ]]; do
      echo -e "\t\t\t\tPlease Enter The Backup You Wish To Restore: \c"
      read bindrestorechoice
  done

  #
  # I'll need to get the full path of the script directory cos I am about to CD for the restore operation
  #

  scriptpath=`realpath $0` # This get the script path + the name, e.g /path/to/script/scriptname.sh
  scriptdir=`dirname $scriptpath` # This get the script directory e.g /path/to/script which is what I need

  echo -e "\t\t\t\tCreating a Temp Directory To Extract The File Into"
  TMPDIR=`mktemp -d /tmp/restore_bind.XXXXXXXXXXXXXXXX` || exit 1

  cd $TMPDIR
  borg extract --list --strip-components $backupbind::$bindrestorechoice 2>> $scriptdir/restore_log/$date.log >/dev/null &

  restorechecker $?

  echo -e "\t\t\t\tMoving Certficate Into Appropriate Folder"
  # Copy Recursively Including Hidden Files
  cp -Rf $TMPDIR/etc/. /etc 2>> $scriptdir/restore_log/$date.log &>/dev/null

  restorechecker $?

  echo -e "\t\t\t\tDone Restoring"
  echo -e "\t\t\t\tRemoving Temp Directory"

  rm -r $TMPDIR

  cd $scriptdir
              
  fi
}

#
#   Function To Restore SFTP Accounts
#

restore_sftp()
{
  backupsftp=/backup/borg/sftp

  date=`date +"%Y-%m-%d"`

  if [[ ! -d "$backupsftp" ]]; then
      echo -e "\t\t\t\tYou can't restore SFTP Users! You either don't have SFTP Accounts or you don't have any backed up"
  else

  echo "
        Input The Date You Want To Restore, e.g, if you want to restore the backup for today ($date),
        you type $date and hit enter
        " | boxes -d columns

  echo -e "\t\t\t\tThis is the back up retained for SFTP"
  echo
  borg list $backupsftp   
  echo

  read -p  $'\t\t\t\t'"Which Backup Do You Wish To Restore " sftprestorechoice

  while [[ $sftprestorechoice = "" ]]; do
      echo -e "\t\t\t\tPlease Enter The Backup You Wish To Restore: \c"
      read sftprestorechoice
  done

  #
  # I'll need to get the full path of the script directory cos I am about to CD for the restore operation
  #

  scriptpath=`realpath $0` # This get the script path + the name, e.g /path/to/script/scriptname.sh
  scriptdir=`dirname $scriptpath` # This get the script directory e.g /path/to/script which is what I need

  echo -e "\t\t\t\tCreating a Temp Directory To Extract The File Into"
  TMPDIR=`mktemp -d /tmp/restore_sftp.XXXXXXXXXXXXXXXX` || exit 1

  cd $TMPDIR
  borg extract --list --strip-components $backupsftp::$sftprestorechoice 2>> $scriptdir/restore_log/$date.log >/dev/null &

  restorechecker $?

  echo -e "\t\t\t\tMoving Certficate Into Appropriate Folder"
  # Copy Recursively Including Hidden Files
  cp -Rf $TMPDIR/sftpusers/jailed/. /sftpusers/jailed 2>> $scriptdir/restore_log/$date.log &>/dev/null
  cp -Rf $TMPDIR/etc/. /etc 2>> $scriptdir/restore_log/$date.log &>/dev/null

  restorechecker $?

  echo -e "\t\t\t\tDone Restoring"
  echo -e "\t\t\t\tRemoving Temp Directory"

  rm -r $TMPDIR

  cd $scriptdir
              
  fi
}

#
#   Function To Let's Encrypt Cert
#

restore_certificate()
{
  backupletsencrypt=/backup/borg/letsencrypt

  date=`date +"%Y-%m-%d"`

  if [[ ! -d "$backupletsencrypt" ]]; then
      echo -e "\t\t\t\tYou can't restore any certificate! You either don't have certificates or you don't have any backed up"
  else

  echo "
        Input The Date You Want To Restore, e.g, if you want to restore the backup for today ($date),
        you type $date and hit enter
        " | boxes -d columns

  echo -e "\t\t\t\tThis is the back up retained for letsencrypt certificate"
  echo
  borg list $backupletsencrypt   
  echo

  read -p  $'\t\t\t\t'"Which Backup Do You Wish To Restore " letsencryptrestorechoice

  while [[ $letsencryptrestorechoice = "" ]]; do
      echo -e "\t\t\t\tPlease Enter The Backup You Wish To Restore: \c"
      read letsencryptrestorechoice
  done

  #
  # I'll need to get the full path of the script directory cos I am about to CD for the restore operation
  #

  scriptpath=`realpath $0` # This get the script path + the name, e.g /path/to/script/scriptname.sh
  scriptdir=`dirname $scriptpath` # This get the script directory e.g /path/to/script which is what I need

  echo -e "\t\t\t\tCreating a Temp Directory To Extract The File Into"
  TMPDIR=`mktemp -d /tmp/restore_letsencrypt.XXXXXXXXXXXXXXXX` || exit 1

  cd $TMPDIR
  borg extract --list --strip-components 1 $backupletsencrypt::$letsencryptrestorechoice etc 2>> $scriptdir/restore_log/$date.log >/dev/null &

  restorechecker $?

  echo -e "\t\t\t\tMoving Certficate Into Appropriate Folder"
  # Copy Recursively Including Hidden Files
  cp -Rf $TMPDIR/letsencrypt/. /etc/letsencrypt 2>> $scriptdir/restore_log/$date.log &>/dev/null

  restorechecker $?

  echo -e "\t\t\t\tDone Restoring"
  echo -e "\t\t\t\tRemoving Temp Directory"

  rm -r $TMPDIR

  cd $scriptdir
              
  fi
}

#
#   Function To Database Files
#

restore_db()
{
  backupdatabase=/backup/borg/database

  date=`date +"%Y-%m-%d"`

  if [[ ! -d "$backupdatabase" ]]; then
      echo -e "\t\t\t\tYou can't restore Databases! You either don't have database or you don't have any backed up"
  else

  echo "
        Input The Date You Want To Restore, e.g, if you want to restore the backup for today ($date),
        you type $date and hit enter
        " | boxes -d columns

  echo -e "\t\t\t\tThis is the back up retained for your Database"
  echo
  borg list $backupdatabase  
  echo

  read -p  $'\t\t\t\t'"Which Backup Do You Wish To Restore " databaserestorechoice

  while [[ $databaserestorechoice = "" ]]; do
      echo -e "\t\t\t\tPlease Enter The Backup You Wish To Restore: \c"
      read databaserestorechoice
  done

  #
  # I'll need to get the full path of the script directory cos I am about to CD for the restore operation
  #

  scriptpath=`realpath $0` # This get the script path + the name, e.g /path/to/script/scriptname.sh
  scriptdir=`dirname $scriptpath` # This get the script directory e.g /path/to/script which is what I need

  echo -e "\t\t\t\tCreating a Temp Directory To Extract The File Into"
  TMPDIR=`mktemp -d /tmp/restore_database.XXXXXXXXXXXXXXXX` || exit 1

  cd $TMPDIR
  borg extract --list $backupdatabase::$databaserestorechoice 2>> $scriptdir/restore_log/$date.log >/dev/null &

  restorechecker $?

  echo -e "\t\t\t\tStoping Mariadb Service"
  sudo systemctl stop mariadb
  echo
  echo -e "\t\t\t\tImporting Database"
  sudo mysql --user root < $TMPDIR/dbs.sql

  restorechecker $?

  echo -e "\t\t\t\tStarting Mariadb Service"
  sudo systemctl start mariadb

  echo -e "\t\t\t\tDone Restoring DB"
  echo -e "\t\t\t\tRemoving Temp Directory"

  rm -r $TMPDIR

  cd $scriptdir
              
  fi
}
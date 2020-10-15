# (CommandLine ControlPanel) Automate Multiple ClassicPress and WordPress Sites + [DNS, PhpMyAdmin, Caching and SSL]
![Automate Multiple Classicpress & Wordpress Installation](https://devsrealm.com/wp-content/uploads/2020/10/CommandLine-Controlpanel.jpg)

If you are tired of using control panels (I am because they are mostly not secure), and want to do everything on the server level, This program handles the automation of ClassicPress, WordPress, and also offers the ability to manage your own DNS directly from the server. 

It also provides the ability to secure your website using Let's Encrypt, and Certbot for the auto-config. I made this optional because I might just want to do some local development, so, I won't need an SSL in that regard.

That is not all, you can also install PhpMyAdmin to manage your Databases from the web, you can add Multiple SFTP Users Interactively, automate the backup (deduplicates + You can configure the frequency of the backup(thanks to borg program)) and restoration of the complete sites (or individual components, e.g bind, SSL Cert, SFTP Users, e.t.c) On The Server, and Many More...


As of the time of writing this, it took only 1min.34s (proof in the tutorial link below) to configure a website on a fresh server instance, it shouldn't take more than 30s to add other sites.

►Key Features

- Install and Configure Multiple Classicpress or WordPress Sites
- Restore an Existing Classicpress or Wordpress Website
- Secure Sites Using Free Let's Encrypt Certificate
- You Can Create More Than one SFTP Users (This Way, You Can Access Through SFTP Client, e,g Filezilla, Winscp, etc)
- Manage Your Own Custom DNS (You can also Add New DNS Zone, Edit Zone, and Even Delete DNS Zone)
- Install PHPMyAdmin (You can also secure it with Let's Encrypt)
- Automate The Backup of the Complete Sites On The Server (De-duplication)
- Edit Backup Retention (How Many Backups Do You Want To Keep In a Day, Week and Month )
- You can Restore The Full Sites or Only DNS (If You Have One Created), Only SFTP Users, Only SSL Cert, etc.
- Ability To Push The Backup To a Cloud Storage (Thanks To Rclone)
- More Features in The Works

►Key Requirements

- Operating system: Ubuntu LTS
- Memory: At least 512MB RAM 
- The installation must be performed on a clean system, free of installed/configured software, especially anything related to web servers.
- The installation must be done as a root user, it won't even allow you to pass through even if you want to.
- If you want to use the Let's Encrypt Feature, make sure you have an A record for the website

## How To Install The Program

The installation is easy:

### Connect to your server as root via SSH

> ssh root@yourserver

### Download The Program, and Change Your Diretory To The Program Directory 

> git clone https://github.com/devsrealm/commandline_controlpanel.git && cd commandline_controlpanel

### Give It an Executable Permission

> chmod +x cp

### Run The Program, and Configure Away

> ./cp yourwebsite.com

*Note: Make Sure You Always Add a Top Level Domain, i.e .com, .net, etc*

First Posted Here: [Automate Multiple ClassicPress & Wordpress Installation](https://devsrealm.com/bash/automate-classicpress-wordpress-installation/)

Reach me at: [Devsrealm](https://devsrealm.com/)

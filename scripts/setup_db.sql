-- Create database
CREATE DATABASE IF NOT EXISTS mirai;
USE mirai;

-- Users table
CREATE TABLE IF NOT EXISTS `users` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(32) NOT NULL,
  `password` varchar(32) NOT NULL,
  `duration_limit` int(10) unsigned DEFAULT NULL,
  `cooldown` int(10) unsigned NOT NULL,
  `wrc` int(10) unsigned DEFAULT NULL,
  `last_paid` int(10) unsigned NOT NULL,
  `max_bots` int(11) DEFAULT '-1',
  `admin` int(10) unsigned DEFAULT '0',
  `intvl` int(10) unsigned DEFAULT '30',
  `api_key` text,
  PRIMARY KEY (`id`),
  KEY `username` (`username`)
);

-- History table
CREATE TABLE IF NOT EXISTS `history` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL,
  `time_sent` int(10) unsigned NOT NULL,
  `duration` int(10) unsigned NOT NULL,
  `command` text NOT NULL,
  `max_bots` int(11) DEFAULT '-1',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`)
);

-- Whitelist table (protected IPs)
CREATE TABLE IF NOT EXISTS `whitelist` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `prefix` varchar(16) DEFAULT NULL,
  `netmask` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `prefix` (`prefix`)
);

-- Insert default admin user
INSERT INTO `users` (`username`, `password`, `admin`, `max_bots`, `duration_limit`, `cooldown`, `last_paid`, `wrc`, `intvl`)
VALUES ('admin', 'admin', 1, -1, 3600, 0, UNIX_TIMESTAMP(), 0, 30);

-- Insert whitelist for your lab network (optional)
-- This prevents attacks on your own infrastructure
INSERT INTO `whitelist` (`prefix`, `netmask`) VALUES ('172.16.193.0', 24);

-- Create database user
GRANT ALL PRIVILEGES ON mirai.* TO 'mirai'@'localhost' IDENTIFIED BY 'miraipass';
FLUSH PRIVILEGES;

-- Show results
SELECT '=== Users ===' AS '';
SELECT id, username, admin, max_bots FROM users;

SELECT '=== Whitelist ===' AS '';
SELECT * FROM whitelist;
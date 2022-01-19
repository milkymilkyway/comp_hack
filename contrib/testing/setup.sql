/* Socket Connection */
CREATE USER 'testuser'@'localhost' IDENTIFIED BY 'un1tt3st';
GRANT ALL PRIVILEGES ON comp_hack_test_lobby.* TO 'testuser'@'localhost';
GRANT ALL PRIVILEGES ON comp_hack_test_world.* TO 'testuser'@'localhost';

/* TCP/IP Connection */
CREATE USER 'testuser'@'%' IDENTIFIED BY 'un1tt3st';
GRANT ALL PRIVILEGES ON comp_hack_test_lobby.* TO 'testuser'@'%';
GRANT ALL PRIVILEGES ON comp_hack_test_world.* TO 'testuser'@'%';


-- ----------------------------
-- Table structure for accounts
-- ----------------------------
DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
  `id` int(6) NOT NULL AUTO_INCREMENT,
  `username` varchar(64) DEFAULT NULL,
  `password` varchar(255) DEFAULT NULL,
  `accesslevel` int(11) DEFAULT '100',
  `lastip` varchar(15) DEFAULT '0.0.0.0',
  `lasttime` int(11) DEFAULT '0',
  `lastsvr` int(11) DEFAULT '0',
  `lastchar` varchar(64) DEFAULT NULL,
  `email` varchar(100) DEFAULT NULL,
  `nb_donation` int(11) DEFAULT '0',
  `donation` varchar(255) DEFAULT '0',
  `active` int(11) DEFAULT '1',
  `active_key` varchar(255) DEFAULT NULL,
  `zulystorage` bigint(11) DEFAULT '0',
  `platinum` tinyint(1) DEFAULT '0',
  `online` tinyint(1) DEFAULT '0',
  `login_count` int(11) DEFAULT '0',
  `isSiteLogged` tinyint(1) DEFAULT '0',
  `CAPTCHA` varchar(255) DEFAULT NULL,
  `access` int(7) DEFAULT NULL,
  `counter` int(7) DEFAULT NULL,
  `v_times` varchar(255) DEFAULT '0',
  `vote_points` bigint(11) unsigned zerofill DEFAULT '00000000000',
  `ExpansionTime` int(11) NOT NULL DEFAULT '0',
  `can_trade` tinyint(1) NOT NULL default '1',
  PRIMARY KEY (`id`),
  KEY `username` (`username`),
  KEY `fb_groupby_lastchar_INDEX` (`lastchar`(10)),
  KEY `fb_tableorder_username_INDEX` (`username`(10))
) ENGINE=InnoDB AUTO_INCREMENT=1005850 DEFAULT CHARSET=latin1;

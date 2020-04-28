-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               10.1.16-MariaDB - mariadb.org binary distribution
-- Server OS:                    Win32
-- HeidiSQL Version:             10.1.0.5464
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping structure for table roseza.accounts
CREATE TABLE IF NOT EXISTS `accounts` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(64) DEFAULT NULL,
  `password` varchar(32) DEFAULT NULL,
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
  `ktpoints` int(11) NOT NULL DEFAULT '0',
  `newpoints` int(11) NOT NULL DEFAULT '0',
  `logtime` int(11) NOT NULL DEFAULT '0',
  `totlogtime` int(11) NOT NULL DEFAULT '0',
  `can_trade` tinyint(1) NOT NULL DEFAULT '1',
  `ExpansionTime` int(11) NOT NULL DEFAULT '0',
  `counter` int(7) NOT NULL DEFAULT '0',
  `vote_points` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=85 DEFAULT CHARSET=latin1;

-- Dumping data for table roseza.accounts: 7 rows
/*!40000 ALTER TABLE `accounts` DISABLE KEYS */;
INSERT INTO `accounts` (`id`, `username`, `password`, `accesslevel`, `lastip`, `lasttime`, `lastsvr`, `lastchar`, `email`, `nb_donation`, `donation`, `active`, `active_key`, `zulystorage`, `platinum`, `online`, `login_count`, `isSiteLogged`, `ktpoints`, `newpoints`, `logtime`, `totlogtime`, `can_trade`, `ExpansionTime`, `counter`, `vote_points`) VALUES
	(78, 'purpleyouko', '80f2ef8d121d9b6dcb5019a5a375db92', 900, '127.0.0.1', 1588094329, 1, 'PurpleYouko', NULL, 0, '0', 1, NULL, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),
	(79, 'Katy', '0571749e2ac330a7455809c6b0e7af90', 900, '65.188.225.108', 1455510537, 1, 'KatysMule', NULL, 0, '0', 1, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),
	(81, 'crystalkuwin', '0ecb88c066fa4cdae8b800f9f217b5c9', 900, '76.4.160.61', 1455506086, 1, 'Saphira', NULL, 0, '0', 1, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),
	(80, 'silveryouko', 'c91cf770bfef6d1160215934427f26d5', 900, '76.4.160.61', 1455489896, 1, 'SilverYouko', NULL, 0, '0', 1, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),
	(82, 'pytest', '5a748c120135eca0627c0c3b4985e917', 900, '127.0.0.1', 1455811462, 1, 'tester1m', NULL, 0, '0', 1, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),
	(83, 'stupid', '80f2ef8d121d9b6dcb5019a5a375db92', 100, '0.0.0.0', 0, 0, NULL, NULL, 0, '0', 1, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),
	(84, 'shaidaran', '80f2ef8d121d9b6dcb5019a5a375db92', 100, '127.0.0.1', 1484670321, 1, 'ShaiLynn', NULL, 0, '0', 1, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0);
/*!40000 ALTER TABLE `accounts` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;

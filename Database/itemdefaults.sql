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

-- Dumping structure for table roseza.itemdefaults
DROP TABLE IF EXISTS `itemdefaults`;
CREATE TABLE IF NOT EXISTS `itemdefaults` (
  `gender` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '0 female, 1 male, 2 neutral',
  `slotnum` int(11) unsigned NOT NULL,
  `itemtype` int(11) unsigned NOT NULL DEFAULT '0',
  `itemnum` int(11) unsigned NOT NULL DEFAULT '0',
  `count` int(11) unsigned DEFAULT '0',
  `refine` int(11) unsigned DEFAULT '0',
  `durability` int(11) unsigned DEFAULT '0',
  `lifespan` int(11) unsigned DEFAULT '0',
  `stats` int(11) unsigned DEFAULT '0',
  `socketed` int(11) unsigned DEFAULT '0',
  `appraised` int(11) unsigned DEFAULT '0',
  KEY `idx_owner` (`gender`),
  KEY `idx_storagetype` (`slotnum`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Item Storage for Characters';

-- Dumping data for table roseza.itemdefaults: ~4 rows (approximately)
/*!40000 ALTER TABLE `itemdefaults` DISABLE KEYS */;
INSERT INTO `itemdefaults` (`gender`, `slotnum`, `itemtype`, `itemnum`, `count`, `refine`, `durability`, `lifespan`, `stats`, `socketed`, `appraised`) VALUES
	(2, 3, 3, 30, 1, 0, 100, 100, 0, 0, 1),
	(2, 6, 5, 30, 1, 0, 100, 100, 0, 0, 1),
	(0, 2, 2, 222, 1, 0, 100, 100, 0, 0, 1),
	(1, 2, 2, 221, 1, 0, 100, 100, 0, 0, 1),
	(2, 7, 8, 1, 1, 0, 100, 100, 0, 0, 1);
/*!40000 ALTER TABLE `itemdefaults` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;

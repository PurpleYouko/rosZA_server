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

-- Dumping structure for procedure roseza.SetDefaultItems
DROP PROCEDURE IF EXISTS `SetDefaultItems`;
DELIMITER //
CREATE DEFINER=`root`@`localhost` PROCEDURE `SetDefaultItems`(
	IN `GenderID` int,
	IN `CharID` int


)
BEGIN
	
SET SQL_SAFE_UPDATES=0;

    START TRANSACTION;
    
    	  	  			      INSERT INTO items 
			(`owner`, `slotnum`, `itemnum`, `itemtype`, `refine`, `durability`, `lifespan`, `count`, `stats`, `socketed`, `appraised`) 
    select 
     CharID, `slotnum`, `itemnum`, `itemtype`, `refine`, `durability`, `lifespan`, `count`, `stats`, `socketed`, `appraised`
    	from itemdefaults where gender=2 or gender=GenderID;
    
        COMMIT;
END//
DELIMITER ;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;

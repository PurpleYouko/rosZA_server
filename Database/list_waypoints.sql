-- phpMyAdmin SQL Dump
-- version 2.10.3
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Feb 20, 2012 at 08:09 AM
-- Server version: 5.0.45
-- PHP Version: 5.2.3

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

-- 
-- Database: `roseza`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `list_waypoints`
-- 

DROP TABLE IF EXISTS `list_waypoints`;
CREATE TABLE IF NOT EXISTS `list_waypoints` (
  `mapid` int(11) NOT NULL,
  `wpnum` int(11) NOT NULL,
  `mapx` float NOT NULL,
  `mapy` float NOT NULL,
  `wptype` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- 
-- Dumping data for table `list_waypoints`
-- 

INSERT INTO `list_waypoints` (`mapid`, `wpnum`, `mapx`, `mapy`, `wptype`) VALUES 
(2, 1, 4953.67, 5532.2, 1),
(2, 2, 4995.67, 5522.8, 2),
(2, 3, 5012.55, 5469.7, 2),
(2, 4, 5073.54, 5438.76, 2),
(2, 5, 5152.83, 5438.61, 2),
(2, 6, 5186.69, 5494.05, 2),
(2, 7, 5178.3, 5519.67, 2),
(2, 8, 5239.18, 5540.66, 2),
(2, 9, 5292.48, 5486.23, 2),
(2, 10, 5373.26, 5518.73, 2),
(2, 11, 5400.48, 5501.71, 2),
(2, 12, 5463.44, 5489.29, 2),
(2, 13, 5510.34, 5516.55, 2),
(2, 14, 5526.38, 5501.96, 2),
(2, 15, 5515.39, 5469.58, 2),
(2, 16, 5515.12, 5373.27, 2),
(2, 17, 5515.02, 5283.29, 3);

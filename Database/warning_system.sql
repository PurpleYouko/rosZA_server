-- phpMyAdmin SQL Dump
-- version 2.10.3
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: May 23, 2017 at 12:48 PM
-- Server version: 5.0.45
-- PHP Version: 5.2.3

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

-- 
-- Database: `roseza`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `warning_system`
-- 

DROP TABLE IF EXISTS `warning_system`;
CREATE TABLE IF NOT EXISTS `warning_system` (
  `account_id` int(6) default NULL,
  `warning_no` tinyint(4) default NULL,
  `time_remaining` int(11) default NULL,
  `ban_type` tinyint(4) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- 
-- Dumping data for table `warning_system`
-- 

INSERT INTO `warning_system` (`account_id`, `warning_no`, `time_remaining`, `ban_type`) VALUES 
(1000291, 1, 0, 0),
(1003817, 1, 0, 0),
(1003769, 1, 0, 0),
(1003710, 1, 0, 0),
(1004629, 1, 0, 0),
(1004531, 1, 0, 0),
(1003713, 1, 0, 0),
(1004432, 1, 0, 0);

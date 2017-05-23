-- phpMyAdmin SQL Dump
-- version 2.10.3
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: May 23, 2017 at 10:44 AM
-- Server version: 5.0.45
-- PHP Version: 5.2.3

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

-- 
-- Database: `roseza`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `map_exp`
-- 

DROP TABLE IF EXISTS `map_exp`;
CREATE TABLE IF NOT EXISTS `map_exp` (
  `id` int(100) NOT NULL auto_increment,
  `map_id` int(100) NOT NULL,
  `map_name` varchar(100) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=26 ;

-- 
-- Dumping data for table `map_exp`
-- 

INSERT INTO `map_exp` (`id`, `map_id`, `map_name`) VALUES 
(1, 1, 'Canyon City of Zant'),
(2, 2, 'City of Junon Polis'),
(3, 21, 'Valley of Luxem Tower'),
(4, 22, 'Adventurer''s Plains'),
(5, 24, 'El Verloon Desert'),
(6, 25, 'Anima Lake'),
(7, 26, 'Forest of Wisdom'),
(8, 27, 'Kenji Beach'),
(9, 28, 'Gorge of Silence'),
(10, 29, 'Desert of the Dead'),
(11, 42, 'Oblivion Temple (B2)'),
(12, 43, 'Oblivion Temple (B3)'),
(13, 81, 'Wasteland Ruins Path'),
(14, 52, 'Mana Snowfields'),
(15, 53, 'Arumic Valley'),
(16, 65, 'Sikuku Underground Prison'),
(17, 62, 'Shady Jungle'),
(18, 63, 'Forest of Wandering'),
(19, 64, 'Marsh of Ghosts'),
(20, 61, 'Xita Refuge'),
(21, 66, 'Sikuku Ruins'),
(22, 78, 'The Wasteland'),
(23, 79, 'The Wasteland'),
(24, 44, 'Oblivion Temple (B4)'),
(25, 81, 'Wasteland Ruins Path');

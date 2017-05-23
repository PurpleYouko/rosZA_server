-- phpMyAdmin SQL Dump
-- version 2.10.3
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: May 23, 2017 at 10:49 AM
-- Server version: 5.0.45
-- PHP Version: 5.2.3

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

-- 
-- Database: `roseza`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `lotto_config`
-- 

DROP TABLE IF EXISTS `lotto_config`;
CREATE TABLE IF NOT EXISTS `lotto_config` (
  `ZulieCost` bigint(11) NOT NULL default '1000000',
  `accesslevel` int(100) NOT NULL default '299',
  `active` int(10) NOT NULL default '0',
  `type` int(10) NOT NULL default '1',
  `ZulieLimit` bigint(11) NOT NULL default '100000000'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- 
-- Dumping data for table `lotto_config`
-- 

INSERT INTO `lotto_config` (`ZulieCost`, `accesslevel`, `active`, `type`, `ZulieLimit`) VALUES 
(1000000, 299, 0, 1, 200000000);

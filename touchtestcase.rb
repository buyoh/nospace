#! /usr/bin/env ruby

require 'fileutils'

Dir.chdir('test') do
  list = Dir.entries('.').select { |path| path[0] != '.' }
  lastNo = list.select { |path| path =~ /^\d{3}/ }.map { |path| path[0..2] }.max
  currNo = lastNo.succ
  FileUtils.touch("#{currNo}_code.txt")
  FileUtils.touch("#{currNo}_stdin.txt")
  FileUtils.touch("#{currNo}_expected.txt")
  open('index.txt', 'a') do |io|
    io.puts "#{currNo}_code.txt #{currNo}_stdin.txt #{currNo}_expected.txt"
  end
end

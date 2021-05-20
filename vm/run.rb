
require_relative './ws.rb'

unless $*.size == 1 || $*.size == 2
    puts "usage: ruby run.rb script-file [stdin-file]"
    exit
end

whitespace = IO.read($*[0])
stdin = $*[1] ? IO.read($*[1]) : STDIN

code, label = WhitespaceCompiler.parse(whitespace)
ip = WhitespaceInterpreter.new(code, label, stdin)
ip.run

exit 0


code = STDIN.read

ws = nil
IO.popen('./maicomp', 'r+') do |io|
    io.print code
    io.flush
    io.close_write
    ws = io.read
end

abort "failed: compilation" if $? != 0

STDOUT.print ws+code.gsub(/\s/, '')

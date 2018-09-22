
def compile(path_in, path_out)
    system "./maicomp < #{path_in} 1> #{path_out} 2> ./temp/cmpstderr.log"
    abort "failed: compilation" if $? != 0
    true
end

def vm_exec(path_code, path_stdin)
    system("ruby ./run.rb ../#{path_code} ../#{path_stdin} 1> .././temp/runstdout.log 2> .././temp/runstderr.log", {chdir: './vm'}) 
end



Dir.mkdir('temp') unless Dir.exist?('temp')

open('./test/index.txt', 'r') do |io|
    while line = io.gets
        code_name, stdin_name, exp_name = line.chomp.split
        puts "test: #{code_name}"
        compile('./test/'+code_name, './temp/out.ws')
        vm_exec('./temp/out.ws', './test/'+stdin_name)
        abort "failed: runtime error(#{$?})" unless $? == 0
        exp = IO.read('./test/'+exp_name)
        act = IO.read('./temp/runstdout.log')
        unless exp == act
            puts "expected:"
            puts exp
            puts "actual:"
            puts act
            abort "failed: assertion"
        end
    end
end

puts "accepted:"

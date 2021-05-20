require 'stringio'

module WhitespaceCompiler
    
    class WhitespaceParseError < RuntimeError
        def initialize(io)
        end
    end
    
    
    def readchar(io, eof = false)
        loop do
            if io.eof?
                return :eof if eof
                raise WhitespaceParseError.new(io)
            end
            c = io.readbyte
            return :sp if c == 32 # 83
            return :tb if c == 9 # 84
            return :lf if c == 10 # 76
        end
    end
    
    
    def parse_int(io)
        sign = 0
        val = 0
        loop do
            c = readchar(io)
            if c == :lf
                raise WhitespaceParseError.new(io) if sign == 0
                return sign*val
            end
            if sign == 0
                sign = c == :tb ? -1 : 1
            else
                val = (val << 1)
                val |= 1 if c == :tb
            end
        end
    end
    
    
    def parse_instruction(io)
        c1 = readchar(io, true)
        return nil if c1 == :eof
        
        if c1 == :sp
            c2 = readchar(io)
            return [:stk_push, parse_int(io)] if c2 == :sp
            c3 = readchar(io)
            return [:stk_copy, parse_int(io)]  if c2 == :tb && c3 == :sp
            return [:stk_slide, parse_int(io)] if c2 == :tb && c3 == :lf
            return [:stk_dup]  if c2 == :lf && c3 == :sp
            return [:stk_swap] if c2 == :lf && c3 == :tb
            return [:stk_del]  if c2 == :lf && c3 == :lf
        elsif c1 == :lf
            c2 = readchar(io)
            c3 = readchar(io)
            return [:flw_label, parse_int(io)] if c2 == :sp && c3 == :sp
            return [:flw_call,  parse_int(io)] if c2 == :sp && c3 == :tb
            return [:flw_jump,  parse_int(io)] if c2 == :sp && c3 == :lf
            return [:flw_jumpz, parse_int(io)] if c2 == :tb && c3 == :sp
            return [:flw_jumpn, parse_int(io)] if c2 == :tb && c3 == :tb
            return [:flw_ret]                  if c2 == :tb && c3 == :lf
            return [:flw_exit]                 if c2 == :lf && c3 == :lf
        elsif c1 == :tb
            c2 = readchar(io)
            if c2 == :sp
                c3 = readchar(io)
                c4 = readchar(io)
                return [:art_add] if c3 == :sp && c4 == :sp
                return [:art_sub] if c3 == :sp && c4 == :tb
                return [:art_mul] if c3 == :sp && c4 == :lf
                return [:art_div] if c3 == :tb && c4 == :sp
                return [:art_mod] if c3 == :tb && c4 == :tb
            elsif c2 == :tb
                c3 = readchar(io)
                return [:hep_store] if c3 == :sp
                return [:hep_ret]   if c3 == :tb
            elsif c2 == :lf
                c3 = readchar(io)
                c4 = readchar(io)
                return [:iop_outc]  if c3 == :sp && c4 == :sp
                return [:iop_outn]  if c3 == :sp && c4 == :tb
                return [:iop_readc] if c3 == :tb && c4 == :sp
                return [:iop_readn] if c3 == :tb && c4 == :tb
            end
        end
        raise WhitespaceParseError.new(io)
    end
    
    
    def parse(io)
        extend WhitespaceCompiler
        io = StringIO.new(io, 'r') if io.is_a? String
        label = {}
        code = []
        
        loop do
            inst = parse_instruction(io)
            break if inst.nil?
            code += inst
            label[inst[1]] = code.size-2 if inst[0] == :flw_label
        end
        return [code, label]
    end
    module_function :parse
end




class WhitespaceInterpreter
    
    class WhitespaceRuntimeError < RuntimeError
        def initialize(wip = '')
            if wip.is_a? WhitespaceInterpreter
                @wip = wip
                msg = "[pc=%d, stack.size=%d, callstack.size=%d]"%[wip.pc, wip.stack.size, wip.callstack.size]
                super msg
            else
                super
            end
        end
    end
    
    def initialize(code, label, stdinio)
        @code = code
        @label = label
        self.stdin = stdinio
        reset()
    end
    
    attr_reader :stdin, :stdout, :pc, :halt, :stack, :heap, :callstack
    def stdin=(s)
        s = StringIO.new(s, 'r') if s.is_a? String
        @stdin = s
    end
    
    def reset
        @pc = 0
        @halt = false
        @stack = []
        @heap = []
        @callstack = []
        self
    end
    
    def step
        return false if @halt
        unless 0 <= @pc && @pc < @code.size
            raise WhitespaceRuntimeError.new(self)
        end
        #p [@code[@pc], @code[@pc+1], @stack, @callstack]
        #abort if @callstack.size >= 4
        self.send(@code[@pc], @code[@pc+1])
        @pc += 1
        true
    end
    
    def run
        while step
        end
        self
    end
    
    
    def getchar
        @stdin.eof? ? -1 : @stdin.readbyte
    end
    
    def getint
        a = @stdin.gets
        a ? a.to_i : 0
    end
    
    
    def stk_push(val)
        @stack.push(val)
        @pc += 1
    end
    def stk_dup(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        @stack.push(@stack[-1])
    end
    def stk_swap(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 2
        @stack[-1], @stack[-2] = @stack[-2], @stack[-1]
    end
    def stk_del(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        @stack.pop()
    end
    def stk_copy(n)
        raise WhitespaceRuntimeError.new(self) if @stack.size < n + 1
        @stack.push(@stack[-n-1])
        @pc += 1
    end
    def stk_slide(n)
        raise WhitespaceRuntimeError.new(self) if @stack.size < n + 1
        @stack = @stack[0, @stack.length-n-1].push(@stack[-1])
        @pc += 1
    end
    
    def flw_label(label)
        # ignore
        @pc += 1
    end
    def flw_call(label)
        pc = @label[label]
        raise WhitespaceRuntimeError.new(self) if pc.nil?
        @callstack.push(@pc+1)
        @pc = pc+1
    end
    def flw_jump(label)
        pc = @label[label]
        raise WhitespaceRuntimeError.new(self) if pc.nil?
        @pc = pc+1
    end
    def flw_jumpz(label)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        @pc += 1
        flw_jump(label) if @stack.pop() == 0
    end
    def flw_jumpn(label)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        @pc += 1
        flw_jump(label) if @stack.pop() < 0
    end
    def flw_ret(_)
        raise WhitespaceRuntimeError.new(self) if @callstack.size < 1
        @pc = @callstack.pop
    end
    def flw_exit(_)
        @halt = true
    end
    
    def art_add(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 2
        a, b = @stack.pop(2)
        @stack.push(a + b)
    end
    def art_sub(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 2
        a, b = @stack.pop(2)
        @stack.push(a - b)
    end
    def art_mul(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 2
        a, b = @stack.pop(2)
        @stack.push(a * b)
    end
    def art_div(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 2
        a, b = @stack.pop(2)
        @stack.push(a / b)
    end
    def art_mod(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 2
        a, b = @stack.pop(2)
        @stack.push(a % b)
    end
    
    def hep_store(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 2
        adr, val = @stack.pop(2)
        @heap[adr] = val
    end
    def hep_ret(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        adr = @stack.pop
        raise WhitespaceRuntimeError.new(self) if @heap[adr].nil?
        @stack.push(@heap[adr])
    end
    
    def iop_outc(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        print "%c"%(@stack.pop)
    end
    def iop_outn(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        print @stack.pop.to_s
    end
    def iop_readc(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        adr = @stack.pop
        @heap[adr] = getchar()
    end
    def iop_readn(_)
        raise WhitespaceRuntimeError.new(self) if @stack.size < 1
        adr = @stack.pop
        @heap[adr] = getint()
    end
    
end


# code, label = WhitespaceCompiler.parse("
# SSSTLSSSTSLTSSSTLSTSSSTSLSSSTLTSSTTLSTSSSTSLSSSTTLTSSLTLSTSSSTTTLSSSTSLTSTSTLSTS
# SSTTTLSSSTSLTSTTTLSTSSTLSSTTLSLSTLSTSLTTLSTSSSTTLSLLTLSTSSSSSSTSTSLTLSSSSSSSTLSS
# STTTLSSSSTSLSSSSSTLTTSTTSSSSSTSLTTTTTTTLSTSSSTSSLLSSSSSSLSLSTLSTSSSTLTSSTSLSLTSS
# SSTLLSLSSSSLLSSSSSTLTLSTSSSTSSLLSSSSTSLSLSTLSTSSSTLTSSTSLSLTTSSTTLLSLSSTSLLSSSST
# TLTLSTSSSSSSSSSTLLSTSTSSLTLSTLLLLSSSTSSLSSSSSSSSTSLTSSSLSTSTSTLSSSSSSTSSSLTSSSLT
# LLSSSTSTLSSSSSSSTSSLTSSSLTL")
# 
# ip = WhitespaceInterpreter.new(code, label, '')
# ip.run
# res = ip.stdout
# 
# if res == "31631-10-1\n74321043210-115"
#     puts "ok"
# else
#     puts "wrong output"
#     puts res
# end



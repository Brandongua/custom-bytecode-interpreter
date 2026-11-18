import sys

SECTION_DATA = 1
SECTION_CODE = 2

registerByteCodes = {
    "a": 0,
    "b": 1,
    "ip": 2,
    "rstatus": 3,
    "rflags": 4
}


labelAddrs = {}
labelData = {}
labelSizes = {}

def printW(msg: str):
    sys.stderr.write(f'\x1b[33m{msg}\x1b[0m\n') #]] stupid bracket matching

#data, type, size
def parseOperand(operand: str, padLabelBytes=8) -> (bytes, str, int):
    if operand[0] == "$":
        operand = operand[1:]
        if operand in labelAddrs:
            data = labelData[operand]
            return data, "LABEL_CONST", labelSizes[operand]
    match operand:
        case REG if REG.lower() in registerByteCodes:
            return registerByteCodes[REG.lower()], "REGISTER", 1

        case NAME if NAME in labelAddrs:
            data = labelAddrs[NAME].to_bytes(labelSizes[NAME], 'little')
            return data, "LABEL", labelSizes[NAME]

        case NAME if NAME.startswith("[") and NAME.endswith("]") and (NAME := NAME.replace("[", "").replace("]", "")) in labelAddrs:
            data = labelAddrs[NAME].to_bytes(8, 'little')
            #size in this case is not the size of the pointer (always 8, its the size of the item that the pointer points to)
            return data, "*LABEL", labelSizes[NAME]

        case NUM if NUM.isnumeric():
            n = int(NUM)
            if n < 0:
                n += (1 << (padLabelBytes * 8))
            return n.to_bytes(padLabelBytes, 'little'), "NUMBER10", padLabelBytes

        case _:
            raise Exception(f"Invalid operand: '{operand}'")

def handle_halt(operands: list[str], **kwargs) -> bytes:
    return b"\x00"

def handle_push(operands: list[str], **kwargs) -> bytes:
    if len(operands) != 1:
        raise Exception("Expected 1 operand for PUSH instruction")
    val, t, size = parseOperand(operands[0])
    b = b"\x01"
    b += val

    return b

def handle_pushr(operands: list[str], **kwargs) -> bytes:
    if len(operands) != 1:
        raise Exception("Expected 1 operand for PUSHR instruction")

    b = b"\x10"
    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    return b

def handle_pushp(operands: list[str], **kwargs) -> bytes:
    if len(operands) != 1:
        raise Exception("Expected 1 operand for PUSHP instruction")
    val, _, size = parseOperand(operands[0])
    if size != 8:
        printW(f"[WARNING]: {operands[0]} is not a 64bit value")
    return b"\x0C" + parseOperand(operands[0])[0]

def handle_pop(operands: list[str], **kwargs) -> bytes:
    return b"\x02"

def handle_goto(operands: list[str], **kwargs) -> bytes:
    b = b"\x06"

    if len(operands) != 1:
        raise Exception("Expected 1 operand for GOTO instruction")

    b += parseOperand(operands[0])[0]
    return b

def handle_jz(operands: list[str], **kwargs) -> bytes:
    b = b"\x0E"
    if len(operands) != 1:
        raise Exception("Expected 1 operand for JZ instruction")

    b += parseOperand(operands[0])[0]
    return b

def handle_jnz(operands: list[str], **kwargs) -> bytes:
    b = b"\x12"
    if len(operands) != 1:
        raise Exception("Expected 1 operand for JNZ instruction")

    b += parseOperand(operands[0])[0]
    return b

def handle_mov(operands: list[str], **kwargs) -> bytes:
    b = b"\x03"
    if len(operands) != 2:
        raise Exception("Expected 2 operands for MOV instruction")

    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')

    val, _, size = parseOperand(operands[1])
    if size != 8:
        printW(f"[WARNING]: mov expects 8 bytes but {operands[1]} is only {size} bytes")

    b += val
    return b

def handle_movr(operands: list[str], **kwargs) -> bytes:
    b = b"\x08"
    if len(operands) != 2:
        raise Exception("Expected 2 operands for MOV instruction")
    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    b += registerByteCodes[operands[1].lower()].to_bytes(1, 'little')
    return b

def handle_preg(operands: list[str], **kwargs) -> bytes:
    b = b"\x04"
    if len(operands) != 1:
        raise Exception("Expected 1 operand for PREG instruction")

    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')

    return b

def handle_pstr(operands: list[str], **kwargs) -> bytes:
    b = b"\x05"
    if len(operands) != 1:
        raise Exception("Expected 1 operand for PREG instruction")

    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')

    return b

def handle_inc(operands: list[str], **kwargs) -> bytes:
    b = b"\x07"
    if len(operands) != 1:
        raise Exception("Expected 1 operand for INC instruction")

    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    return b

def handle_dec(operands: list[str], **kwargs) -> bytes:
    b = b"\x0D"
    if len(operands) != 1:
        raise Exception("Expected 1 operands for DEC instruction")

    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    return b

def handle_add(operands: list[str], **kwargs) -> bytes:
    b = b"\x09"
    if len(operands) != 2:
        raise Exception("Expected 2 operands for ADD instruction")
    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    if operands[1].isnumeric():
        b += b"\x02"
        b += int(operands[1]).to_bytes(8, 'little')
    elif operands[1].lower() in registerByteCodes:
        b += b"\x01"
        b += registerByteCodes[operands[1].lower()].to_bytes(1, 'little')
    return b

def handle_adds(operands: list[str], **kwargs) -> bytes:
    return b"\x0B"


def handle_sub(operands: list[str], **kwargs) -> bytes:
    b = b"\x0F"
    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    b += registerByteCodes[operands[1].lower()].to_bytes(1, 'little')
    return b

def handle_cmp(operands: list[str], **kwargs) -> bytes:
    b = b"\x1A"
    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    b += registerByteCodes[operands[1].lower()].to_bytes(1, 'little')
    return b

def handle_rea(operands: list[str], **kwargs) -> bytes:
    b = b"\x11"
    if len(operands) != 3:
        raise Exception("Expected 3 operands for the REA instruction")
    val, _, size = parseOperand(operands[0], 1)
    if size != 1:
        raise Exception(f"Cannot use {operands[0]} as a size is it is not 1 byte")
    b += val
    b += registerByteCodes[operands[1].lower()].to_bytes(1, 'little')
    b += registerByteCodes[operands[2].lower()].to_bytes(1, 'little')
    return b

def handle_malloc(operands: list[str], **kwargs) -> bytes:
    b = b"\x13"
    if len(operands) != 1:
        raise Exception("Expected 1 operands for MALLOC instruction")
    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    return b

def handle_free(operands: list[str], **kwargs) -> bytes:
    b = b"\x14"
    if len(operands) != 1:
        raise Exception("Expected 1 operands for FREE instruction")
    b += registerByteCodes[operands[0].lower()].to_bytes(1, 'little')
    return b

def handle_movp(operands: list[str], **kwargs) -> bytes:
    b = b"\x15"
    if len(operands) != 3:
        raise Exception("Expected 3 operands for the MOVP instruction")

    val, _, size = parseOperand(operands[0], 1)
    if size != 1:
        raise Exception(f"Cannot use {operands[0]} as a size is it is not 1 byte")
    b += val
    b += registerByteCodes[operands[1].lower()].to_bytes(1, 'little')
    b += registerByteCodes[operands[2].lower()].to_bytes(1, 'little')
    return b

def handle_call(operands: list[str], **kwargs) -> bytes:
    return b'\x16'

def handle_ret(*args, **kwargs) -> bytes:
    return b'\x17'

def handle_retz(*args, **kwargs) -> bytes:
    return b'\x18'

def handle_revs(*args, **kwargs) -> bytes:
    return b'\x19'

instructionHandlers = {
        "halt": handle_halt,
        "push": handle_push,
        "pushp": handle_pushp,
        "pushr": handle_pushr,
        "mov": handle_mov,
        "preg": handle_preg,
        "pstr": handle_pstr,
        "pop": handle_pop,
        "goto": handle_goto,
        "jz": handle_jz,
        "jnz": handle_jnz,
        "inc": handle_inc,
        "dec": handle_dec,
        "movr": handle_movr,
        'add': handle_add,
        'adds': handle_adds,
        "sub": handle_sub,
        "nop": lambda *args: b'\x20',
        "rea": handle_rea,
        "malloc": handle_malloc,
        "free": handle_free,
        "movp": handle_movp,
        "call": handle_call,
        "ret": handle_ret,
        "retz": handle_retz,
        "revs": handle_revs,
        "cmp": handle_cmp,
}

def sizeToInt(size: str):
    match size:
        case "BYTE":
            return 1
        case "SHORT":
            return 2
        case "INT":
            return 4
        case "LONG":
            return 8

def assembleDataLine(line: str, offsetFromStart: int) -> bytes:
    line = line.strip()
    words = line.split(" ")
    name = words[0]
    size = sizeToInt(words[1])
    b = b""
    for valNo, val in enumerate(words[2].split(",")):
        val = int(val)
        if val < 0:
            val += (1 << size * 8)
        value = int(val).to_bytes(size, 'little')
        #the label should point to the first value
        if valNo == 0:
            labelData[name] = value
        b += value
    labelAddrs[name] = offsetFromStart
    labelSizes[name] = size

    return b

def assembleCodeLine(line: str, offsetFromStart: int) -> bytes:
    line = line.strip()

    if line.endswith(":"):
        lName = line[:-1]
        labelAddrs[lName] = offsetFromStart
        labelSizes[lName] = 8
        return b""

    words = line.split(" ")
    instruction = words[0].lower()
    if len(words) > 1:
        operands = "".join(words[1:]).split(",")
    else:
        operands = []
    return instructionHandlers[instruction](operands)

def assembleLine(line: str, section: int, offsetFromStart: int) -> bytes:
    if section == SECTION_DATA:
        return assembleDataLine(line, offsetFromStart)
    return assembleCodeLine(line, offsetFromStart)

def assemble(text: str):
    lines = text.split("\n")

    section = 0

    outBytes = b""
    
    offsetFromStart = 8

    codeStartOffset = 0

    for line in lines:
        line = line.strip()
        semiIndex = line.find(";")
        if semiIndex != -1:
            line = line[:semiIndex]
        if not line: continue
        if line == "section .data":
            section = SECTION_DATA
        elif line == "section .code":
            section = SECTION_CODE
            codeStartOffset = offsetFromStart
            # outBytes = (codeStartOffset + 8).to_bytes(8, 'little') + outBytes
        else:
            b = assembleLine(line, section, offsetFromStart)
            offsetFromStart += len(b)
            outBytes += b
    entryPointOffset = labelAddrs.get("_start")
    if entryPointOffset is None:
        raise Exception("Could not find _start entry point")
    outBytes = entryPointOffset.to_bytes(8, 'little') + outBytes
    return outBytes

if len(sys.argv) < 2:
    print("No file name given")
    exit(1)

with open(sys.argv[1], "r") as f:
    text = f.read()

with open(f"{sys.argv[1]}.out", "wb") as f:
    f.write(assemble(text))

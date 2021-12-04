meta:
  id: lwsc
  file-extension: conf
  title: lwsc-configfile
  endian: le
seq:
  - id: array_length
    type: u1
  - id: machinedate
    type: machinedata
    repeat: expr
    repeat-expr: array_length
    
types:
  machinedata:
    seq:
      - id: name
        type: str
        size: 38
        encoding: ASCII
      - id: shortname
        type: str
        size: 9
        encoding: ASCII
      - id: id
        type: u4
      - id: disabled
        type: u1
      - id: symbol_x
        type: s4
      - id: symbol_y
        type: s4
      - id: function
        type: machinefunction
        repeat: expr
        repeat-expr: 5
  
  machinefunction:
    seq:
      - id: name
        type: str
        size: 38
        encoding: ASCII
      - id: duration
        type: s4
      - id: bitmask
        type: u1
      - id: symbol_x
        type: u4
      - id: symbol_y
        type: u4
      - id: rotation
        type: u1
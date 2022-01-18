from xml.etree.ElementTree import Element
from xml.etree.ElementTree import SubElement
from xml.etree.ElementTree import ElementTree

def prettify(element, indent, newline, level = 0): 
    if element:    
        if element.text == None or element.text.isspace():
            element.text = newline + indent * (level + 1)
        else:    
            element.text = newline + indent * (level + 1) + element.text.strip()\
                         + newline + indent * (level + 1)
    temp = list(element)
    for subelement in temp:
        if temp.index(subelement) < (len(temp) - 1):
            subelement.tail = newline + indent * (level + 1)
        else:
            subelement.tail = newline + indent * level
        prettify(subelement, indent, newline, level = level + 1)

# declare a tag and add text.
def SubElementText(_parent,_tag, attrib={}, _text=None, **_extra):
    result = SubElement(_parent,_tag,attrib,**_extra)
    result.text = _text
    return result

# list to string
def l2s(input):
    output = str(input[0])
    for i in input[1:]:
        output += ' ' + str(i)

    return output
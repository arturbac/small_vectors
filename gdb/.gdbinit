python
import sys
import gdb
import traceback

def member_exists(gdb_value, member_name):
    try:
        gdb_value[member_name]
        return True
    except gdb.error:
        return False
        
class small_vectors:
    class BasicStringPrinter:
        def __init__(self, val):
            self.val = val

        def to_string(self):
            try:
                # print("BasicStringPrinter.to_string called")
                char_type = self.val.type.template_argument(0)
                # print(f"char_type: {char_type}")

                storage = self.val['storage_']
                data = storage['data_']
                size = int(storage['size_'])
                
                if member_exists(storage, 'active_'):
                  # print("storage has 'active_' attribute")
                  active = int(storage['active_'])
                  if active == 0:  # Assuming 0 is for buffered
                      # print("Using buffered storage")
                      char_data = data['buffered']['_M_elems']
                  else:
                      # print("Using dynamic storage")
                      char_data = data['dynamic']['data']
                else:
                  # print("storage does not have 'active_' attribute")
                  char_data = data['_M_elems']
                # print(f"size: {size}")

                # Get the string content
                if size > 0:
                    result = f'"{char_data.string(length=size)}"'
                else:
                    result = '""'

                # print(f"result: {result}")
                return result
            except Exception as e:
                print(f"Exception in BasicStringPrinter.to_string: {str(e)}")
                traceback.print_exc()
                return f"<error accessing string: {str(e)}>"

def basic_string_lookup_function(val):
    try:
        # print(f"basic_string_lookup_function called with type: {val.type}")
        type_name = str(val.type.strip_typedefs())
        # print(f"type_name: {type_name}")
        if 'small_vectors::v3_0::basic_string_t<' in type_name:
            # print("Returning BasicStringPrinter")
            return small_vectors.BasicStringPrinter(val)
        # print("No matching pretty-printer found")
    except Exception as e:
        print(f"Exception in basic_string_lookup_function: {str(e)}")
        traceback.print_exc()
    return None


try:
    #sys.path.insert(0, '/home/artur/projects/small_vectors/gdb')
    #print("Python path updated")
    # import basic_string_printer
    gdb.pretty_printers.append(basic_string_lookup_function)

except Exception as e:
    print("Error occurred:")
    print(str(e))
    print("Traceback:")
    traceback.print_exc()
end

set print pretty on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set demangle-style gnu-v3
set print sevenbit-strings off

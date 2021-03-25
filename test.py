from cracker.csv_util import read_csv
from cracker.data_util import is_data_valid
from cracker.static import Static


static = Static()
read_csv(static, f_name='experiments/data/logic_data.csv')


print(is_data_valid(static))
select a.date, a.size, a.address, a.file, a.line, d.date, d.file, d.line
from allocation as a
left join deallocation as d on a.id = d.allocation_id
where a.session_id = 3

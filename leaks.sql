select allocation.* from allocation
left join deallocation on allocation.id = deallocation.allocation_id
where allocation.session_id = 3 and deallocation.id is null

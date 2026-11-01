COLS=10; ROWS=8  # D题 10×8 栅格
import math

class Node:
    def __init__(self, i, j):
        self.i = i  # 网格列索引 col (0-9，共10列)
        self.j = j  # 网格行索引 row (0-7，共8行)
        self.g = 0  # 起点到当前的代价
        self.h = 0  # 到目标的估计代价
        self.f = 0  # f = g + h
        self.parent = None

    def __eq__(self, other):
        return self.i == other.i and self.j == other.j

    def __repr__(self):
        return f"Node({self.i},{self.j})"


def a_star_single(start_i, start_j, end_i, end_j, obstacles):
    """单段A*：从起点到终点的最短路径（不考虑遍历）"""
    open_list = []
    closed_list = []
    start_node = Node(start_i, start_j)
    end_node = Node(end_i, end_j)
    open_list.append(start_node)

    while open_list:
        # 找f值最小的节点
        current_idx = 0
        current_node = open_list[0]
        for idx, node in enumerate(open_list):
            if node.f < current_node.f:
                current_node = node
                current_idx = idx

        open_list.pop(current_idx)
        closed_list.append(current_node)

        # 到达终点
        if current_node == end_node:
            path = []
            current = current_node
            while current:
                path.append((current.i, current.j))
                current = current.parent
            return path[::-1]  # 反转路径

        # 生成邻居：只上下左右，禁止斜向移动
        neighbors = []
        for di, dj in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
            ni, nj = current_node.i + di, current_node.j + dj

            # 检查是否在网格内且非禁飞区
            if 0 <= ni <= COLS-1 and 0 <= nj <= ROWS-1 and (ni, nj) not in obstacles:
                neighbors.append(Node(ni, nj))

        # 处理邻居
        for neighbor in neighbors:
            if neighbor in closed_list:
                continue

            # 计算移动代价（直线0.5m）
            neighbor.g = current_node.g + 0.5

            # 启发函数（直线距离）
            dx = (end_i - neighbor.i) * 0.5
            dy = (end_j - neighbor.j) * 0.5
            neighbor.h = math.sqrt(dx**2 + dy**2)
            neighbor.f = neighbor.g + neighbor.h
            neighbor.parent = current_node

            # 加入开放列表（已存在且代价更高则跳过）
            if not any(n == neighbor and n.g <= neighbor.g for n in open_list):
                open_list.append(neighbor)

    return None  # 无路径


def get_all_non_obstacle(obstacles):
    """获取所有非禁飞区网格坐标"""
    all_grids = [(i, j) for i in range(COLS) for j in range(ROWS)]  # 9x7网格
    return [grid for grid in all_grids if grid not in obstacles]


def cover_all_grids(start_i, start_j, obstacles):
    """规划覆盖所有非禁飞区的最短路径（优化目标选择，减少方向变化）"""
    non_obstacles = get_all_non_obstacle(obstacles)
    if not non_obstacles:
        return []
    
    # 初始化：从起点开始，标记已访问
    visited = set()
    current_path = []
    current_i, current_j = start_i, start_j
    visited.add((current_i, current_j))
    current_path.append((current_i, current_j))
    
    # 记录当前移动方向（初始为None）
    current_dir = None  # 方向向量(i_diff, j_diff)

    # 循环直到覆盖所有非禁飞区
    while len(visited) < len(non_obstacles):
        min_cost = float('inf')
        target = None
        
        # 遍历所有未访问网格，计算综合代价（距离+方向一致性权重）
        for grid in non_obstacles:
            if grid not in visited:
                # 基础代价：直线距离
                dist = math.hypot((grid[0]-current_i)*0.5, (grid[1]-current_j)*0.5)
                
                # 方向一致性权重：如果与当前方向一致，降低代价
                dir_penalty = 0
                if current_dir is not None:
                    # 计算目标相对于当前位置的方向
                    target_dir = (grid[0] - current_i, grid[1] - current_j)
                    # 标准化方向（确保方向向量为单位步长）
                    norm_target_dir = (
                        0 if target_dir[0] == 0 else target_dir[0] / abs(target_dir[0]),
                        0 if target_dir[1] == 0 else target_dir[1] / abs(target_dir[1])
                    )
                    # 方向不一致时增加惩罚代价（使方向一致的目标更优）
                    if norm_target_dir != current_dir:
                        dir_penalty = 0.3  # 惩罚系数，可调整
                
                # 综合代价 = 距离 + 方向惩罚
                total_cost = dist + dir_penalty
                
                # 选择综合代价最小的目标
                if total_cost < min_cost:
                    min_cost = total_cost
                    target = grid

        # 从当前位置到目标的最短路径
        segment = a_star_single(current_i, current_j, target[0], target[1], obstacles)
        if not segment:
            return None  # 无法到达目标，无完整路径
        
        # 更新当前方向（根据新路径的整体方向）
        if len(segment) >= 2:
            first_step = (segment[1][0] - segment[0][0], segment[1][1] - segment[0][1])
            current_dir = (
                0 if first_step[0] == 0 else first_step[0] / abs(first_step[0]),
                0 if first_step[1] == 0 else first_step[1] / abs(first_step[1])
            )
        
        # 添加路径（排除重复的起点）
        for grid in segment[1:]:
            current_path.append(grid)
            visited.add(grid)
        
        # 更新当前位置
        current_i, current_j = target

    return current_path


def get_coordinates(i, j):
    """将网格索引转换为实际坐标(m)：方格中心坐标"""
    # 网格(0,0)对应实际(0, 0)
    x = i * 0.5  # x轴从左到右（网格i增大方向）
    y = j * 0.5  # y轴从下到上（网格j增大方向）
    return (round(x, 2), round(y, 2))


def extract_waypoints(full_path):
    """从完整路径中提取拐点（连续三点a→b→c中，若a→b与b→c方向不同，则b为拐点）"""
    if len(full_path) <= 2:
        return full_path  # 路径太短，直接返回所有点
    
    waypoints = [full_path[0]]  # 起点一定是拐点
    
    # 遍历路径找方向变化的点（检查连续三个点）
    for k in range(1, len(full_path) - 1):
        # 获取连续三个点a, b, c
        a = full_path[k-1]
        b = full_path[k]
        c = full_path[k+1]
        
        # 计算方向向量a→b和b→c
        dir_ab = (b[0] - a[0], b[1] - a[1])
        dir_bc = (c[0] - b[0], c[1] - b[1])
        
        # 方向不同时，b为拐点
        if dir_ab != dir_bc:
            waypoints.append(b)
    
    waypoints.append(full_path[-1])  # 终点一定是拐点
    return waypoints


def validate_end_points(ends, obstacles):
    """验证终点是否合法（在网格内且非禁飞区）"""
    valid_ends = []
    for (x, y) in ends:
        if 0 <= x <= COLS-1 and 0 <= y <= ROWS-1 and (x, y) not in obstacles:
            valid_ends.append((x, y))
    return valid_ends


def select_best_end_path(start_i, start_j, end_points, obstacles):
    """从多个终点中选择拐点最少的路径"""
    best_path = None
    best_waypoints = None
    best_end = None
    min_waypoint_count = float('inf')
    
    for end in end_points:
        end_i, end_j = end
        # 规划到该终点的路径
        path = a_star_single(start_i, start_j, end_i, end_j, obstacles)
        if not path:
            continue  # 路径不存在，跳过
        
        # 提取拐点
        waypoints = extract_waypoints(path)
        waypoint_count = len(waypoints)
        
        # 更新最优路径
        if waypoint_count < min_waypoint_count:
            min_waypoint_count = waypoint_count
            best_path = path
            best_waypoints = waypoints
            best_end = end
    
    return best_waypoints, best_end


def plan_route_v2(start_x, start_y, obstacles=None, end_points=None):
    """
    V2版本：支持可变数量禁飞区

    参数:
        start_x, start_y: 起点网格坐标
        obstacles: [(i,j), ...] 禁飞区列表，可变长度
        end_points: [(i,j), ...] 候选终点列表，None=全覆盖模式

    返回:
        grid_waypoints, real_waypoints, end_x, end_y
    """
    if obstacles is None:
        obstacles = []

    # 检查起点合法性
    def is_valid(grid):
        i, j = grid
        return 0 <= i <= COLS-1 and 0 <= j <= ROWS-1 and grid not in obstacles

    start_grid = (start_x, start_y)
    if not is_valid(start_grid):
        return None, None, None, None

    if end_points:
        valid_ends = validate_end_points(end_points, obstacles)
        if not valid_ends:
            return None, None, None, None
        grid_waypoints, best_end = select_best_end_path(start_x, start_y, valid_ends, obstacles)
        if not grid_waypoints or not best_end:
            return None, None, None, None
        real_waypoints = [get_coordinates(i, j) for (i, j) in grid_waypoints]
        return grid_waypoints, real_waypoints, best_end[0], best_end[1]
    else:
        full_grid_path = cover_all_grids(start_x, start_y, obstacles)
        if not full_grid_path:
            return None, None, None, None
        grid_waypoints = extract_waypoints(full_grid_path)
        real_waypoints = [get_coordinates(i, j) for (i, j) in grid_waypoints]
        return grid_waypoints, real_waypoints, None, None


# ── 原版兼容 ──
def plan_route(start_x, start_y, obs_y0, obs_x0, obs_y1, obs_x1, obs_y2, obs_x2,
               end1_x=None, end1_y=None, end2_x=None, end2_y=None, end3_x=None, end3_y=None):
    """
    主函数：规划路径并返回拐点的网格坐标和实际坐标
    参数：
        start_x, start_y: 起点网格坐标
        obs_y0, obs_x0...: 三个禁飞区的网格坐标 (y, x)
        end1_x, end1_y...: 三个终点的网格坐标（后6个参数）
    返回：
        grid_waypoints: 拐点的网格坐标列表
        real_waypoints: 拐点的实际坐标列表（单位m）
        end_x_1: 选中的终点网格x坐标（终点模式）或None（全覆盖模式）
        end_y_1: 选中的终点网格y坐标（终点模式）或None（全覆盖模式）
    """
    # 构建禁飞区列表（注意参数顺序是y, x对应i, j）
    obstacles = [
        (obs_x0, obs_y0),  # (i, j) = (x, y)
        (obs_x1, obs_y1),
        (obs_x2, obs_y2)
    ]
    
    # 检查起点合法性
    def is_valid(grid):
        i, j = grid
        return 0 <= i <= COLS-1 and 0 <= j <= ROWS-1 and grid not in obstacles
    
    start_grid = (start_x, start_y)
    if not is_valid(start_grid):
        return None, None, None, None
    
    # 检查禁飞区合法性
    for obs in obstacles:
        if not (0 <= obs[0] <= COLS-1 and 0 <= obs[1] <= ROWS-1):
            return None, None, None, None
    
    # 判断模式：终点选择模式（6个终点参数均不为None） vs 全覆盖模式
    end_params = [end1_x, end1_y, end2_x, end2_y, end3_x, end3_y]
    is_end_mode = all(param is not None for param in end_params)
    
    if is_end_mode:
        # 终点选择模式：处理三个终点
        end_points = [
            (end1_x, end1_y),
            (end2_x, end2_y),
            (end3_x, end3_y)
        ]
        # 验证终点合法性
        valid_ends = validate_end_points(end_points, obstacles)
        if not valid_ends:
            return None, None, None, None
        
        # 选择拐点最少的路径
        grid_waypoints, best_end = select_best_end_path(start_x, start_y, valid_ends, obstacles)
        if not grid_waypoints or not best_end:
            return None, None, None, None
        
        # 转换为实际坐标
        real_waypoints = [get_coordinates(i, j) for (i, j) in grid_waypoints]
        return grid_waypoints, real_waypoints, best_end[0], best_end[1]
    else:
        # 全覆盖模式：原有逻辑
        full_grid_path = cover_all_grids(start_x, start_y, obstacles)
        if not full_grid_path:
            return None, None, None, None
        
        # 提取拐点
        grid_waypoints = extract_waypoints(full_grid_path)
        # 转换为实际坐标
        real_waypoints = [get_coordinates(i, j) for (i, j) in grid_waypoints]
        return grid_waypoints, real_waypoints, None, None


# 示例调用
if __name__ == "__main__":
    # 示例1：全覆盖模式（后6个参数为None）
    print("=== 全覆盖模式示例 ===")
    grid_wp, real_wp, end_x, end_y = plan_route(
        start_x=0, start_y=0,
        obs_y0=2, obs_x0=5,
        obs_y1=3, obs_x1=5,
        obs_y2=4, obs_x2=5,
        end1_x=None, end1_y=None, end2_x=None, end2_y=None, end3_x=None, end3_y=None
    )
    if grid_wp and real_wp:
        print("拐点网格坐标：", grid_wp)
        print("拐点实际坐标：", real_wp)
        print("终点：无（全覆盖模式）\n")
    else:
        print("全覆盖模式路径规划失败\n")

    # 示例2：终点选择模式（提供三个终点）
    print("=== 终点选择模式示例 ===")
    grid_wp, real_wp, end_x, end_y = plan_route(
        start_x=0, start_y=0,
        obs_y0=2, obs_x0=5,
        obs_y1=3, obs_x1=5,
        obs_y2=4, obs_x2=5,
        end1_x=8, end1_y=6,  # 终点1
        end2_x=8, end2_y=0,  # 终点2
        end3_x=0, end3_y=6   # 终点3
    )
    if grid_wp and real_wp and end_x is not None and end_y is not None:
        print("拐点网格坐标：", grid_wp)
        print("拐点实际坐标：", real_wp)
        print(f"选中的终点网格坐标：({end_x}, {end_y})")
        print(f"选中的终点实际坐标：{get_coordinates(end_x, end_y)}")
        print(f"拐点数量：{len(grid_wp)}")
    else:
        print("终点选择模式路径规划失败")

































# import math

# class Node:
#     def __init__(self, i, j):
#         self.i = i  # 网格x索引(0-8，共9行)
#         self.j = j  # 网格y索引(0-6，共7列)
#         self.g = 0  # 起点到当前的代价
#         self.h = 0  # 到目标的估计代价
#         self.f = 0  # f = g + h
#         self.parent = None

#     def __eq__(self, other):
#         return self.i == other.i and self.j == other.j

#     def __repr__(self):
#         return f"Node({self.i},{self.j})"


# def a_star_single(start_i, start_j, end_i, end_j, obstacles):
#     """单段A*：从起点到终点的最短路径（不考虑遍历）"""
#     open_list = []
#     closed_list = []
#     start_node = Node(start_i, start_j)
#     end_node = Node(end_i, end_j)
#     open_list.append(start_node)

#     while open_list:
#         # 找f值最小的节点
#         current_idx = 0
#         current_node = open_list[0]
#         for idx, node in enumerate(open_list):
#             if node.f < current_node.f:
#                 current_node = node
#                 current_idx = idx

#         open_list.pop(current_idx)
#         closed_list.append(current_node)

#         # 到达终点
#         if current_node == end_node:
#             path = []
#             current = current_node
#             while current:
#                 path.append((current.i, current.j))
#                 current = current.parent
#             return path[::-1]  # 反转路径

#         # 生成邻居（完善禁飞区相邻方格的斜向移动限制）
#         neighbors = []
#         current_pos = (current_node.i, current_node.j)
        
#         for di in [-1, 0, 1]:
#             for dj in [-1, 0, 1]:
#                 if di == 0 and dj == 0:
#                     continue  # 跳过自身
                
#                 # 计算新位置
#                 ni, nj = current_node.i + di, current_node.j + dj
#                 target_pos = (ni, nj)
                
#                 # 检查是否为斜向移动
#                 is_diagonal = (di != 0) and (dj != 0)
                
#                 # 如果是斜向移动，检查当前位置或目标位置是否在禁飞区的上下左右相邻位置
#                 if is_diagonal:
#                     # 检查当前节点是否紧邻禁飞区（上下左右）
#                     current_adjacent = any(
#                         (current_pos == (oi + 1, oj)) or  # 禁飞区下方
#                         (current_pos == (oi - 1, oj)) or  # 禁飞区上方
#                         (current_pos == (oi, oj + 1)) or  # 禁飞区右方
#                         (current_pos == (oi, oj - 1))     # 禁飞区左方
#                         for (oi, oj) in obstacles
#                     )
                    
#                     # 检查目标节点是否紧邻禁飞区（上下左右）
#                     target_adjacent = any(
#                         (target_pos == (oi + 1, oj)) or  # 禁飞区下方
#                         (target_pos == (oi - 1, oj)) or  # 禁飞区上方
#                         (target_pos == (oi, oj + 1)) or  # 禁飞区右方
#                         (target_pos == (oi, oj - 1))     # 禁飞区左方
#                         for (oi, oj) in obstacles
#                     )
                    
#                     # 如果当前位置或目标位置是禁飞区相邻方格，禁止斜向移动
#                     if current_adjacent or target_adjacent:
#                         continue  # 禁飞区相邻方格上禁止斜向移动
                
#                 # 检查是否在网格内且非禁飞区
#                 if 0 <= ni <= COLS-1 and 0 <= nj <= ROWS-1 and (ni, nj) not in obstacles:
#                     neighbors.append(Node(ni, nj))

#         # 处理邻居
#         for neighbor in neighbors:
#             if neighbor in closed_list:
#                 continue

#             # 计算移动代价（斜向√0.5m，直线0.5m）
#             if abs(neighbor.i - current_node.i) == 1 and abs(neighbor.j - current_node.j) == 1:
#                 neighbor.g = current_node.g + math.sqrt(0.5)
#             else:
#                 neighbor.g = current_node.g + 0.5

#             # 启发函数（直线距离）
#             dx = (end_i - neighbor.i) * 0.5
#             dy = (end_j - neighbor.j) * 0.5
#             neighbor.h = math.sqrt(dx**2 + dy**2)
#             neighbor.f = neighbor.g + neighbor.h
#             neighbor.parent = current_node

#             # 加入开放列表（已存在且代价更高则跳过）
#             if not any(n == neighbor and n.g <= neighbor.g for n in open_list):
#                 open_list.append(neighbor)

#     return None  # 无路径


# def get_all_non_obstacle(obstacles):
#     """获取所有非禁飞区网格坐标"""
#     all_grids = [(i, j) for i in range(COLS) for j in range(ROWS)]  # 9x7网格
#     return [grid for grid in all_grids if grid not in obstacles]


# def cover_all_grids(start_i, start_j, obstacles):
#     """规划覆盖所有非禁飞区的最短路径（优化目标选择，减少方向变化）"""
#     non_obstacles = get_all_non_obstacle(obstacles)
#     if not non_obstacles:
#         return []
    
#     # 初始化：从起点开始，标记已访问
#     visited = set()
#     current_path = []
#     current_i, current_j = start_i, start_j
#     visited.add((current_i, current_j))
#     current_path.append((current_i, current_j))
    
#     # 记录当前移动方向（初始为None）
#     current_dir = None  # 方向向量(i_diff, j_diff)

#     # 循环直到覆盖所有非禁飞区
#     while len(visited) < len(non_obstacles):
#         min_cost = float('inf')
#         target = None
        
#         # 遍历所有未访问网格，计算综合代价（距离+方向一致性权重）
#         for grid in non_obstacles:
#             if grid not in visited:
#                 # 基础代价：直线距离
#                 dist = math.hypot((grid[0]-current_i)*0.5, (grid[1]-current_j)*0.5)
                
#                 # 方向一致性权重：如果与当前方向一致，降低代价
#                 dir_penalty = 0
#                 if current_dir is not None:
#                     # 计算目标相对于当前位置的方向
#                     target_dir = (grid[0] - current_i, grid[1] - current_j)
#                     # 标准化方向（确保方向向量为单位步长）
#                     norm_target_dir = (
#                         0 if target_dir[0] == 0 else target_dir[0] / abs(target_dir[0]),
#                         0 if target_dir[1] == 0 else target_dir[1] / abs(target_dir[1])
#                     )
#                     # 方向不一致时增加惩罚代价（使方向一致的目标更优）
#                     if norm_target_dir != current_dir:
#                         dir_penalty = 0.3  # 惩罚系数，可调整
                
#                 # 综合代价 = 距离 + 方向惩罚
#                 total_cost = dist + dir_penalty
                
#                 # 选择综合代价最小的目标
#                 if total_cost < min_cost:
#                     min_cost = total_cost
#                     target = grid

#         # 从当前位置到目标的最短路径
#         segment = a_star_single(current_i, current_j, target[0], target[1], obstacles)
#         if not segment:
#             return None  # 无法到达目标，无完整路径
        
#         # 更新当前方向（根据新路径的整体方向）
#         if len(segment) >= 2:
#             first_step = (segment[1][0] - segment[0][0], segment[1][1] - segment[0][1])
#             current_dir = (
#                 0 if first_step[0] == 0 else first_step[0] / abs(first_step[0]),
#                 0 if first_step[1] == 0 else first_step[1] / abs(first_step[1])
#             )
        
#         # 添加路径（排除重复的起点）
#         for grid in segment[1:]:
#             current_path.append(grid)
#             visited.add(grid)
        
#         # 更新当前位置
#         current_i, current_j = target

#     return current_path


# def get_coordinates(i, j):
#     """将网格索引转换为实际坐标(m)：方格中心坐标"""
#     # 网格(0,0)对应实际(0, 0)
#     x = i * 0.5  # x轴从左到右（网格i增大方向）
#     y = j * 0.5  # y轴从下到上（网格j增大方向）
#     return (round(x, 2), round(y, 2))


# def extract_waypoints(full_path):
#     """从完整路径中提取拐点（新定义：连续三点a→b→c中，若a→b与b→c方向不同，则b为拐点）"""
#     if len(full_path) <= 2:
#         return full_path  # 路径太短，直接返回所有点
    
#     waypoints = [full_path[0]]  # 起点一定是拐点
    
#     # 遍历路径找方向变化的点（检查连续三个点）
#     for k in range(1, len(full_path) - 1):
#         # 获取连续三个点a, b, c
#         a = full_path[k-1]
#         b = full_path[k]
#         c = full_path[k+1]
        
#         # 计算方向向量a→b和b→c
#         dir_ab = (b[0] - a[0], b[1] - a[1])
#         dir_bc = (c[0] - b[0], c[1] - b[1])
        
#         # 方向不同时，b为拐点
#         if dir_ab != dir_bc:
#             waypoints.append(b)
    
#     waypoints.append(full_path[-1])  # 终点一定是拐点
#     return waypoints


# def plan_route(start_x, start_y, obstacles_x1, obstacles_y1, obstacles_x2, obstacles_y2, obstacles_x3, obstacles_y3):
#     """
#     主函数：规划路径并返回拐点的网格坐标和实际坐标
#     参数：
#         start_x, start_y: 起点网格坐标
#         obstacles_x1, obstacles_y1...: 三个禁飞区的网格坐标
#     返回：
#         grid_waypoints: 拐点的网格坐标列表
#         real_waypoints: 拐点的实际坐标列表（单位m）
#     """
#     # 构建禁飞区列表
#     obstacles = [
#         (obstacles_x1, obstacles_y1),
#         (obstacles_x2, obstacles_y2),
#         (obstacles_x3, obstacles_y3)
#     ]
    
#     # 检查起点和禁飞区是否在合法范围内
#     def is_valid(grid):
#         i, j = grid
#         return 0 <= i <= COLS-1 and 0 <= j <= ROWS-1
    
#     start_grid = (start_x, start_y)
#     if not is_valid(start_grid) or start_grid in obstacles:
#         return None, None
    
#     for obs in obstacles:
#         if not is_valid(obs):
#             return None, None
    
#     # 规划完整路径
#     full_grid_path = cover_all_grids(start_x, start_y, obstacles)
#     if not full_grid_path:
#         return None, None
    
#     # 提取拐点
#     grid_waypoints = extract_waypoints(full_grid_path)
    
#     # 转换为实际坐标
#     real_waypoints = [get_coordinates(i, j) for (i, j) in grid_waypoints]
    
#     return grid_waypoints, real_waypoints


# # 示例调用
# # if __name__ == "__main__":
# #     # 示例参数：起点网格坐标(0,0)，三个禁飞区网格坐标
# #     grid_waypoints, real_waypoints = plan_route(
# #         start_x=0, start_y=0,
# #         obstacles_x1=5, obstacles_y1=2,
# #         obstacles_x2=5, obstacles_y2=3,
# #         obstacles_x3=5, obstacles_y3=4
# #     )
    
# #     if grid_waypoints and real_waypoints:
# #         print("拐点网格坐标：")
# #         for wp in grid_waypoints:
# #             print(wp, end=" → " if wp != grid_waypoints[-1] else "\n")
        
# #         print("\n拐点实际坐标（单位：m）：")
# #         for wp in real_waypoints:
# #             print(wp, end=" → " if wp != real_waypoints[-1] else "\n")
        
# #         print(f"\n总拐点数量：{len(grid_waypoints)}")
# #     else:
# #         print("无法规划有效路径")

# ── D题: 计算三个目标中离出口最远的 ──
def farthest_target(targets, exit_col, exit_row, obstacles):
    """targets=[(col,row,color),...], 返回(farthest_col, farthest_row, path)"""
    best_path=None; best_target=None; best_len=-1
    for tc,tr,_ in targets:
        path=a_star_single(tc,tr,exit_col,exit_row,obstacles)
        if path and len(path)>best_len:
            best_len=len(path);best_path=path;best_target=(tc,tr)
    return best_target,best_path

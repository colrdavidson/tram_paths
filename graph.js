"use strict";

var mouse_x = 0;
var mouse_y = 0;
var node_radius = 5;
var conn_width = 5;
var active_conn = null;
var first_conn = true;

function draw_line(ctx, x1, y1, x2, y2, width) {
	ctx.beginPath();
	ctx.moveTo(x1, y1);
	ctx.lineTo(x2, y2);
	ctx.lineWidth = width;
	ctx.stroke();
}

class Node {
	constructor(x, y, color) {
		this.x = x;
		this.y = y;
		this.color = color;
		this.conn = [];
	}
}

class Connection {
	constructor(a, b, color) {
		this.a = a;
		this.b = b;
		this.color = color;
	}
}

class Bus {
	constructor(conn, node, color) {
		this.conn = conn;
		this.color = color;

		if (conn.a == node) {
			this.dir = 1;
		} else if (conn.b == node) {
			this.dir = -1;
		} else {
			this.dir = 0;
		}

		this.x = node.x;
		this.y = node.y;
		this.angle = Math.atan2(conn.b.y - conn.a.y, conn.b.x - conn.a.x);
	}

	calc_angle() {
		return Math.atan2(this.conn.b.y - this.conn.a.y, this.conn.b.x - this.conn.a.x);
	}
}

function get_conn(conn_lookup, node1, node2) {
	var lookup_str = node1.x + "|" + node1.y + "|" + node2.x + "|" + node2.y;
	return conn_lookup[lookup_str];
}

function insert_conn(conn_lookup, conn) {
	var lookup_str1 = conn.a.x + "|" + conn.a.y + "|" + conn.b.x + "|" + conn.b.y;
	var lookup_str2 = conn.b.x + "|" + conn.b.y + "|" + conn.a.x + "|" + conn.a.y;
	conn_lookup[lookup_str1] = conn;
	conn_lookup[lookup_str2] = conn;
}

function reverse_conn(conn) {
	var tmp_node = conn.a;
	conn.a = conn.b;
	conn.b = tmp_node;
}

function walk(nodes, visited, cycle_stack, node, parent) {
	var node_str = node.x + ", " + node.y;
	visited[node_str] = true;

	for (var i = 0; i < node.conn.length; i++) {
		var tmp_str = node.conn[i].x + ", " + node.conn[i].y;
		if (visited[tmp_str] == false) {
			if (walk(nodes, visited, cycle_stack, node.conn[i], node)) {
				return true;
			}
		} else if (node.conn[i] != parent) {
			return true;
		}
	}

	return false;
}

function detect_cycle(nodes) {
	var visited = {};
	var cycle_stack = [];
	for (var i = 0; i < nodes.length; i++) {
		var tmp_str = nodes[i].x + ", " + nodes[i].y;
		visited[tmp_str] = false;
	}

	for (var i = 0; i < nodes.length; i++) {
		var tmp_str = nodes[i].x + ", " + nodes[i].y;
		if (visited[tmp_str] == false) {
			if (walk(nodes, visited, cycle_stack, nodes[i], null)) {
				return true;
			}
		}
	}
	return false;
}

function draw_node(ctx, x, y) {
	ctx.beginPath();
	ctx.arc(x, y, node_radius, 0, Math.PI * 2, false);
	ctx.fill();
}

function mouse_moved(canvas, event, nodes) {
	var rect = canvas.getBoundingClientRect();
	mouse_x = event.clientX - rect.left;
	mouse_y = event.clientY - rect.top;
}

function mouse_pressed(canvas, event, nodes, connections, conn_lookup, buses) {
	var rect = canvas.getBoundingClientRect();
	mouse_x = event.clientX - rect.left;
	mouse_y = event.clientY - rect.top;

	if (event.button == 0) { // left click
		var node_hit = false;
		for (var i = 0; i < nodes.length; i++) {
			var node_x = nodes[i].x;
			var node_y = nodes[i].y;

			var circle_test = Math.sqrt(Math.pow(Math.abs(mouse_x - node_x), 2) + Math.pow(Math.abs(mouse_y - node_y), 2));

			if (circle_test < node_radius) {
				if (active_conn == null) {
					active_conn = nodes[i];
				} else if (active_conn != nodes[i]) {
					// Check for prexisting connection
					var connection_exists = false;
					for (var j = 0; j < connections.length; j++) {
						if (((connections[j].a == active_conn) && (connections[j].b == nodes[i])) ||
							((connections[j].a == nodes[i]) && (connections[j].b == active_conn))) {
							connection_exists = true;
						}
					}

					if (!connection_exists) {
						var tmp_conn = new Connection(active_conn, nodes[i], 'blue');
						connections.push(tmp_conn);
						insert_conn(conn_lookup, tmp_conn);
						active_conn.conn.push(nodes[i]);
						nodes[i].conn.push(active_conn);

						if (first_conn) {
							buses.push(new Bus(tmp_conn, active_conn, 'red'));
							buses.push(new Bus(tmp_conn, active_conn, 'red'));
							buses.push(new Bus(tmp_conn, active_conn, 'red'));
							first_conn = false;
						}
					}
					active_conn = nodes[i];
				}
				node_hit = true;
			}
		}

		if (!node_hit) {
			active_conn = null;
		}
	} else if (event.button == 2) { // right click
		for (var i = 0; i < nodes.length; i++) {
			var node_x = nodes[i].x;
			var node_y = nodes[i].y;

			var circle_test = Math.sqrt(Math.pow(Math.abs(mouse_x - node_x), 2) + Math.pow(Math.abs(mouse_y - node_y), 2));

			if (circle_test < node_radius) {
				return;
			}
		}

		var tmp = new Node(mouse_x, mouse_y, 'white');
		nodes.push(tmp);

		if (active_conn != null) {
			var tmp_conn = new Connection(active_conn, tmp, 'blue');
			connections.push(tmp_conn);
			insert_conn(conn_lookup, tmp_conn);
			active_conn.conn.push(tmp);
			tmp.conn.push(active_conn);

			if (first_conn) {
				buses.push(new Bus(tmp_conn, active_conn, 'red'));
				buses.push(new Bus(tmp_conn, active_conn, 'red'));
				buses.push(new Bus(tmp_conn, active_conn, 'red'));
				first_conn = false;
			}
		}
		active_conn = tmp;
	}
}

function lerp(x1, x2, t) {
	return x1 + t * (x2 - x1);
}

function render(ctx, lerpy, nodes, connections, conn_lookup, buses) {
	ctx.fillStyle = 'black';
	ctx.fillRect(0, 0, 800, 800);

	ctx.strokeStyle = 'green';
	if (active_conn != null) {
		draw_line(ctx, active_conn.x, active_conn.y, mouse_x, mouse_y, conn_width);
	}

	for (var i = 0; i < connections.length; i++) {
		ctx.strokeStyle = connections[i].color;
		draw_line(ctx, connections[i].a.x, connections[i].a.y, connections[i].b.x, connections[i].b.y, conn_width);
	}

	for (var i = 0; i < buses.length; i++) {
		var bus = buses[i];
        ctx.fillStyle = bus.color;
		var bus_angle = bus.angle;
		ctx.save();
		ctx.translate(bus.x, bus.y);
		ctx.rotate(bus.angle);
		ctx.fillRect(0, 0, 20, 10);
		ctx.restore();

		var new_x = lerp(bus.x, bus.conn.b.x, lerpy);
		var new_y = lerp(bus.y, bus.conn.b.y, lerpy);
		if (lerpy >= 1) {
			var reversed = false;
			var old_b = bus.conn.b;
			if (bus.conn.b.conn.length > 2) {
				var next_station_idx = Math.floor(Math.random() * bus.conn.b.conn.length);
				while (bus.conn.b == bus.conn.b.conn[next_station_idx]) {
					next_station_idx = Math.floor(Math.random() * bus.conn.b.conn.length);
				}

				buses[i].conn = Object.assign({}, get_conn(conn_lookup, bus.conn.b, bus.conn.b.conn[next_station_idx]));
			} else if (bus.conn.b.conn.length == 2) {
				if (bus.conn.b.conn[0] == bus.conn.a) {
					buses[i].conn = Object.assign({}, get_conn(conn_lookup, bus.conn.b, bus.conn.b.conn[1]));
				} else {
					buses[i].conn = Object.assign({}, get_conn(conn_lookup, bus.conn.b, bus.conn.b.conn[0]));
				}
			} else {
				reversed = true;
				reverse_conn(buses[i].conn);
			}

			if (old_b == buses[i].conn.b && !reversed) {
				reverse_conn(buses[i].conn);
			}

			buses[i].x = bus.conn.a.x;
			buses[i].y = bus.conn.a.y;
		} else {
			buses[i].x = new_x;
			buses[i].y = new_y;
		}

		buses[i].angle = bus.calc_angle();
	}

	for (var i = 0; i < nodes.length; i++) {
		ctx.fillStyle = nodes[i].color;
		draw_node(ctx, nodes[i].x, nodes[i].y);
		var tmp_str = nodes[i].x + ", " + nodes[i].y;
		ctx.fillText(tmp_str, nodes[i].x, nodes[i].y - (node_radius * 2));
	}
}

function start_graph() {
	var canvas = document.getElementById('canvas');
	if (canvas.getContext) {
		var ctx = canvas.getContext('2d');

		var width = 800;
		var height = 800;

		var nodes = [];
		var connections = [];
		var conn_lookup = { };
		var buses = [];

		canvas.addEventListener("mousemove", function(evt) { mouse_moved(canvas, evt, nodes); }, false);
		canvas.addEventListener("mousedown", function(evt) { mouse_pressed(canvas, evt, nodes, connections, conn_lookup, buses); }, false);
		canvas.oncontextmenu = function (evt) { evt.preventDefault(); };


		var last_time = Date.now();
		var dt = 0;
		var lerpy = 0;
		function draw_graph(step) {
			var time_now = Date.now();
			dt = (time_now - last_time) / 1000;
			last_time = time_now;

			lerpy += dt;
			render(ctx, lerpy, nodes, connections, conn_lookup, buses);
			if (lerpy >= 1) {
				lerpy = 0;
			}
			window.requestAnimationFrame(draw_graph);
		}

		window.requestAnimationFrame(draw_graph);
	}
}

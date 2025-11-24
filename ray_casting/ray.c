/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ray.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: omaezzem <omaezzem@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/11 12:52:51 by omaezzem          #+#    #+#             */
/*   Updated: 2025/11/14 13:06:23 by omaezzem         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../cub.h"
#include <math.h>

void	init_player_raycasting(t_cub *cub)
{
	char	o = cub->map_lines[cub->player_pos.y][cub->player_pos.x];

	cub->player.pos_x = cub->player_pos.x + 25;
	cub->player.pos_y = cub->player_pos.y + 25;

	if (o == 'N')
		cub->player.angle = 3 * M_PI / 2; // (3 * 180) / 2 = 270
 	else if (o == 'S')
		cub->player.angle = M_PI / 2; // 180 / 2 = 90
	else if (o == 'E')
		cub->player.angle = 0; // 0
	else if (o == 'W')
		cub->player.angle = M_PI; // 180 
	else
	{
		ft_putstr_fd("Error: invalid player orientation\n", 2);
		exit(EXIT_FAILURE);
	}
}

void	load_textures(t_cub *cub)
{
	cub->tex_north.img = NULL;
	cub->tex_south.img = NULL;
	cub->tex_east.img = NULL;
	cub->tex_west.img = NULL;
	printf("Skipping textures - using solid colors\n");
}

void	load_texture(t_cub *cub, t_texture *tex, char *path)
{
	tex->img = mlx_xpm_file_to_image(cub->mlx, path, 
		&tex->width, &tex->height);
	if (!tex->img)
	{
		ft_putstr_fd("Error\nFailed to load texture: ", 2);
		ft_putstr_fd(path, 2);
		ft_putstr_fd("\n", 2);
		exit(EXIT_FAILURE);
	}
	tex->addr = mlx_get_data_addr(tex->img, &tex->bpp, 
		&tex->line_len, &tex->endian);
}

int is_wall(t_cub *cub, double x, double y)
{
    int map_x = (int)(x / T_SIZE);
    int map_y = (int)(y / T_SIZE);

    if (map_x < 0 || map_y < 0 ||
        map_x >= cub->size_x / T_SIZE ||
        map_y >= cub->size_y / T_SIZE)
        return 1;
    return (cub->map_lines[map_y][map_x] == '1');
}

double ft_calcul_vitaghors(t_cub *cub, double hit_x, double hit_y)
{
    double dx = hit_x - cub->player.pos_x;
    double dy = hit_y - cub->player.pos_y;

    return sqrt(dx * dx + dy * dy);
}

double h_check(t_cub *cub, double *hit_x, double *hit_y)
{
    double x;
    double y;
    double check_y;

    cub->player.intersection_y = floor(cub->player.pos_y / T_SIZE) * T_SIZE;
    if (cub->player.f_down)
        cub->player.intersection_y += T_SIZE;
    cub->player.intersection_x = cub->player.pos_x +
                    (cub->player.intersection_y - cub->player.pos_y) / tan(cub->player.angle);
    x = cub->player.intersection_x;
    y = cub->player.intersection_y;
    cub->player.stp_y = T_SIZE;
    if (cub->player.f_up)
        cub->player.stp_y = -T_SIZE;
    cub->player.stp_x = cub->player.stp_y / tan(cub->player.angle);
    while (x >= 0 && x <= cub->size_x && y >= 0 && y <= cub->size_y)
    {
		if (cub->player.f_up)
			check_y = y - 1;
		else
			check_y = y;
        if (is_wall(cub, x, check_y))
        {
            *hit_x = x;
            *hit_y = y;
            return ft_calcul_vitaghors(cub, x, y);
        }
        x += cub->player.stp_x;
        y += cub->player.stp_y;
    }
    return INT_MAX;
}


double v_check(t_cub *cub, double *hit_x, double *hit_y)
{
	double x;
    double y;
    double check_y;

    cub->player.intersection_x = floor(cub->player.pos_x / T_SIZE) * T_SIZE;
    if (cub->player.f_right)
        cub->player.intersection_x += T_SIZE;
    cub->player.intersection_y = cub->player.pos_y +
                    (cub->player.intersection_x - cub->player.pos_x) / tan(cub->player.angle);
    x = cub->player.intersection_x;
    y = cub->player.intersection_y;
    cub->player.stp_x = T_SIZE;
    if (cub->player.left)
        cub->player.stp_x = -T_SIZE;
    cub->player.stp_y = cub->player.stp_x / tan(cub->player.angle);
    while (x >= 0 && x <= cub->size_x && y >= 0 && y <= cub->size_y)
    {
		if (cub->player.f_left)
			check_x = x - 1;
		else
			check_x = x;
        if (is_wall(cub, x, check_y))
        {
            *hit_x = x;
            *hit_y = y;
            return ft_calcul_vitaghors(cub, x, y);
        }
        x += cub->player.stp_x;
        y += cub->player.stp_y;
    }
    return INT_MAX;
}

double	ft_casting(t_cub *cub)
{
	cub->cast.h_hitx = 0;
	cub->cast.h_hity = 0;
	cub->cast.v_hitx = 0;
	cub->cast.v_hity = 0;
	cub->cast.h_distance = h_check(cub, &cub->cast.h_hitx, &cub->cast.h_hity);
	cub->cast.v_distance = v_check(cub, &cub->cast.h_hity, &cub->cast.v_hity);
	if (cub->cast.h_distance > cub->cast.v_distance)
	{
		cub->player.closer_dst_x = cub->cast.h_hitx;
		cub->player.closer_dst_y = cub->cast.h_hity;
		cub->player.v_ok = true;
		return (cub->cast.v_distance);
	}
	else
	{
		cub->player.closer_dst_x = cub->cast.v_hitx;
		cub->player.closer_dst_y = cub->cast.v_hity;
		cub->player.h_ok = true;
		return (cub->cast.h_distance);
	}
}

void	draw_frame(t_cub *cub)
{
	int		i;
	t_ray	ray;

	cub->img = mlx_new_image(cub->mlx, WIDTH, HEIGHT);
	cub->addr = mlx_get_data_addr(cub->img, &cub->bpp, &cub->line_len, &cub->endian);
	draw_ceiling(cub);
	draw_floor(cub);
	ray.distance_projection_plan = (WIDTH / 2 / tan(FOV / 2));

	i = 0;
	cub->player.f_down = false;
	cub->player.f_up = false;
	cub->player.f_right = false;
	cub->player.f_left = false;
	ray.s_angle = cub->player.angle - (FOV / 2); // this if the player facing example 270 the start angle is 270 - (60 / 2) = 240
	ray.o_angle = cub->player.angle;
	while (i < WIDTH)
	{
		ray.update_angle = cub->ray.s_angle + i * (FOV / WIDTH);
		if (cub->player.angle > M_PI * 2)
			cub->player.angle = fmod(cub->player.angle, 2 * M_PI);
		else if(cub->player.angle < 0)
			cub->player.angle += 2 * M_PI;
		if (cub->player.angle > M_PI / 2 && cub->player.angle < (3 * M_PI / 2))
			cub->player.f_left = true;
		else if (cub->player.angle < M_PI / 2 && cub->player.angle > (3 * M_PI / 2))
			cub->player.f_right = true;
		else if (cub->player.angle > 0 && cub->player.angle < M_PI)
			cub->player.f_down = 1;
		else
			cub->player.f_up = 1;
		ray.distance_r = ft_casting(cub);
		ray.feye_distance = cos(ray.update_angle - ray.o_angle) * ray.distance_r;
		ray.h_wall_projection = (T_SIZE / ray.feye_distance) * ray.distance_projection_plan;
		
	}
	mlx_put_image_to_window(cub->mlx, cub->window, cub->img, 0, 0);
	mlx_destroy_image(cub->mlx, cub->img);
}
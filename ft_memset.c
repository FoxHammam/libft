/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hhommani <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/27 10:10:51 by hhommani          #+#    #+#             */
/*   Updated: 2026/10/28 15:34:11 by hhommani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "libft.h"

void	*ft_memset(void *s, int c, size_t n)
{
	size_t			i;
	unsigned char	*p;

	i = 0;
	p = (unsigned char *)s;
	while (i < n)
	{
		*p = (unsigned char)c;
		p++;
		i++;
	}
	return (s);
}
/*

#include <stdio.h>
int main ()
{
	int b;
	int a = -2004;
	unsigned char *p1 = (unsigned char *) &a;
	unsigned char *p2 = (unsigned char *) &b;
	int i = 0;
	while(i < sizeof(int))
	{
		ft_memset(p2,*p1, 1);
		p2++;
		p1++;
		i++;
	}
	printf("%d",b);
}
	*/
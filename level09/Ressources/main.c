/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arsciand <arsciand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/17 15:35:24 by arsciand          #+#    #+#             */
/*   Updated: 2022/05/17 15:44:04 by arsciand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if (argc == 2)
  {
    for (size_t i = 0; argv[1][i] != '\0'; i++)
      dprintf(STDOUT_FILENO, "%c", (argv[1][i] - (char)i));
    dprintf(STDOUT_FILENO, "\n");
    return (EXIT_SUCCESS);
  }
  else
  {
    dprintf(STDERR_FILENO, "USAGE:\n\t./a.out <string to decode>\n");
    return (EXIT_FAILURE);
  }
}
